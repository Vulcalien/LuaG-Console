/* Copyright 2022-2023 Vulcalien
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "terminal.h"

#include "display.h"
#include "shell-commands.h"
#include "data-structs/char-queue.h"
#include "data-structs/circular-array.h"

#include <string.h>

#include <SDL2/SDL.h>

// colors
#define TERM_COLOR_NORMAL (0xffffff)
#define TERM_COLOR_ERROR  (0xff0000)
#define TERM_COLOR_INPUT  (0x00ff00)
#define CURSOR_COLOR      (0xff0000)

#define ROWS_IN_DISPLAY\
    ((DISPLAY_HEIGHT - 2) / (CHAR_HEIGHT + LINE_SPACING))
#define CHARS_IN_ROW\
    ((DISPLAY_WIDTH - 2) / (CHAR_WIDTH + LETTER_SPACING))

#define ANIMATION_DELAY (25)

#define MAX_LINE_LEN (127)
#define MAX_CLOSED_ROWS (2048)

static struct {
    char *text;
    u32 len;
    u32 cursor_pos;

    enum {
        LINE_TYPE_NORMAL,
        LINE_TYPE_ERROR,
        LINE_TYPE_INPUT
    } type;
} active_line;

struct row {
    char text[CHARS_IN_ROW + 1];
    u32 color;
};

static i32 scroll_position = 0;
static i32 cursor_animation_ticks = 0;

// closed rows
static struct row *closed_rows;
static i32 closed_rows_count = 0;

// command history
static struct CircularArray *command_history;
static i32 history_index = -1;

// BUFFERS
static struct CharQueue *user_buffer;
static struct CharQueue *output_buffer;

static void terminal_set_scroll(i32 scroll);

static void terminal_execute(void);

static void allocate_active_line(void);

int terminal_init(void) {
    allocate_active_line();

    closed_rows     = calloc(MAX_CLOSED_ROWS, sizeof(struct row));
    command_history = circulararray_create(1024);

    user_buffer   = charqueue_create(4096);
    output_buffer = charqueue_create(4096);

    return 0;
}

void terminal_destroy(void) {
    free(active_line.text);
    free(closed_rows);

    circulararray_destroy(command_history, free);

    charqueue_destroy(user_buffer);
    charqueue_destroy(output_buffer);
}

static inline void close_row(void) {
    closed_rows_count++;

    if(closed_rows_count == MAX_CLOSED_ROWS) {
        // overwrite the first half of the closed lines
        memcpy(
            closed_rows,
            closed_rows + MAX_CLOSED_ROWS / 2,
            (MAX_CLOSED_ROWS / 2) * sizeof(struct row)
        );

        // delete second half
        memset(
            closed_rows + MAX_CLOSED_ROWS / 2,
            0,
            (MAX_CLOSED_ROWS / 2) * sizeof(struct row)
        );

        closed_rows_count = MAX_CLOSED_ROWS / 2;
    }
}

static void close_active_line(void) {
    // split and save the active line into closed_rows
    struct row *current_row;
    for(u32 i = 0; i < active_line.len; i++) {
        if(i % CHARS_IN_ROW == 0) {
            if(i > 0)
                close_row();
            current_row = &closed_rows[closed_rows_count];

            switch(active_line.type) {
                case LINE_TYPE_NORMAL:
                    current_row->color = TERM_COLOR_NORMAL;
                    break;
                case LINE_TYPE_ERROR:
                    current_row->color = TERM_COLOR_ERROR;
                    break;
                case LINE_TYPE_INPUT:
                    current_row->color = TERM_COLOR_INPUT;
                    break;
            }
        }
        current_row->text[i % CHARS_IN_ROW] = active_line.text[i];
    }
    close_row();
}

void terminal_tick(void) {
    char c;

    if(!charqueue_is_empty(output_buffer)) {
        c = charqueue_dequeue(output_buffer);
        if(active_line.type == LINE_TYPE_INPUT)
            active_line.type = LINE_TYPE_NORMAL;
    } else if(!charqueue_is_empty(user_buffer)) {
        c = charqueue_dequeue(user_buffer);
        active_line.type = LINE_TYPE_INPUT;
    } else {
        // all buffers are empty
        cursor_animation_ticks++;
        return;
    }

    cursor_animation_ticks = 0;

    if(c == '\n') {
        close_active_line();

        if(active_line.type == LINE_TYPE_INPUT)
            terminal_execute();

        allocate_active_line();
    } else if(c == '\x0b') {
        // error line indicator
        active_line.type = LINE_TYPE_ERROR;
    } else if(c == '\b') {
        if(active_line.cursor_pos != 0) {
            if(active_line.cursor_pos != active_line.len) {
                memmove(
                    active_line.text + active_line.cursor_pos - 1,
                    active_line.text + active_line.cursor_pos,
                    active_line.len - active_line.cursor_pos
                );
            }
            active_line.cursor_pos--;
            active_line.len--;
            active_line.text[active_line.len] = '\0';
        }
    } else if(c == '\x7f') {
        if(active_line.cursor_pos != active_line.len) {
            if(active_line.cursor_pos < active_line.len - 1) {
                memmove(
                    active_line.text + active_line.cursor_pos,
                    active_line.text + active_line.cursor_pos + 1,
                    active_line.len - active_line.cursor_pos
                );
            }
            active_line.len--;
            active_line.text[active_line.len] = '\0';
        }
    } else if(c == '\x11') {
        // up key
        if(history_index + 1 < circulararray_count(command_history)) {
            history_index++;

            char *command = circulararray_get(
                command_history, history_index
            );
            strcpy(active_line.text, command);
            active_line.len = strlen(active_line.text);
            active_line.cursor_pos = active_line.len;
        }
    } else if(c == '\x12') {
        // left key
        if(active_line.cursor_pos != 0)
            active_line.cursor_pos--;
    } else if(c == '\x13') {
        // down key
        if(history_index > 0) {
            history_index--;

            char *command = circulararray_get(
                command_history, history_index
            );
            strcpy(active_line.text, command);
            active_line.len = strlen(active_line.text);
            active_line.cursor_pos = active_line.len;
        } else if(history_index == 0) {
            history_index--;

            // clear active line
            strcpy(active_line.text, "");
            active_line.len = 0;
            active_line.cursor_pos = 0;
        }
    } else if(c == '\x14') {
        // right key
        if(active_line.cursor_pos != active_line.len)
            active_line.cursor_pos++;
    } else if(c == '\x15') {
        // ctrl+c
        for(u32 i = 0; i < 2; i++) {
            close_active_line();
            allocate_active_line();
        }
    } else if(c == '\x16') {
        // ctrl+shift+c
        SDL_SetClipboardText(active_line.text);
    } else if(c == '\x17') {
        // ctrl+shift+v
        char *text = SDL_GetClipboardText();
        terminal_receive_input(text);
        SDL_free(text);
    } else if(c == '\x18') {
        // ctrl+u

        // clear active line
        strcpy(active_line.text, "");
        active_line.len = 0;
        active_line.cursor_pos = 0;
    } else if(c == '\x19') {
        // ctrl+w or ctrl+backspace
        // TODO ctrl+w or ctrl+backspace
    } else if(c == '\x1a') {
        // ctrl+del
        // TODO ctrl+del
    } else if(c >= ' ' && c <= '~') {
        if(active_line.len != MAX_LINE_LEN) {
            if(active_line.cursor_pos != active_line.len) {
                memmove(
                    active_line.text + active_line.cursor_pos + 1,
                    active_line.text + active_line.cursor_pos,
                    active_line.len - active_line.cursor_pos
                );
            }
            active_line.text[active_line.cursor_pos] = c;
            active_line.cursor_pos++;

            active_line.len++;
            active_line.text[active_line.len] = '\0';
        }
    }

    terminal_set_scroll(closed_rows_count - ROWS_IN_DISPLAY + 1);
}

void terminal_render(void) {
    display_clear(0x000000);

    u32 rendered_lines = 0;
    for(u32 i = scroll_position; i < closed_rows_count; i++) {
        display_write(
            closed_rows[i].text, closed_rows[i].color,
            1, 1 + (CHAR_HEIGHT + LINE_SPACING) * rendered_lines,
            1, 0xff
        );

        rendered_lines++;
        if(rendered_lines >= ROWS_IN_DISPLAY)
            break;
    }

    if(rendered_lines < ROWS_IN_DISPLAY) {
        u32 color;
        switch(active_line.type) {
            case LINE_TYPE_NORMAL:
                color = TERM_COLOR_NORMAL;
                break;
            case LINE_TYPE_ERROR:
                color = TERM_COLOR_ERROR;
                break;
            case LINE_TYPE_INPUT:
                color = TERM_COLOR_INPUT;
                break;
        }
        display_write(
            active_line.text, color,
            1, 1 + (CHAR_HEIGHT + LINE_SPACING) * rendered_lines,
            1, 0xff
        );

        if((active_line.type == LINE_TYPE_INPUT ||
            cursor_animation_ticks != 0) &&
           cursor_animation_ticks / ANIMATION_DELAY % 2 == 0) {
            display_write(
                "_", CURSOR_COLOR,
                1 + (CHAR_WIDTH + LETTER_SPACING) * active_line.cursor_pos,
                1 + (CHAR_HEIGHT + LINE_SPACING) * rendered_lines,
                1, 0xff
            );
        }
    }
}

void terminal_receive_input(const char *c) {
    for(u32 i = 0; c[i] != '\0'; i++) {
        if(charqueue_is_full(user_buffer))
            break;
        charqueue_enqueue(user_buffer, c[i]);
    }
}

static void terminal_set_scroll(i32 scroll) {
    if(scroll > ((i32) closed_rows_count) - ROWS_IN_DISPLAY + 1)
        scroll = closed_rows_count - ROWS_IN_DISPLAY + 1;

    if(scroll < 0)
        scroll = 0;

    scroll_position = scroll;
}

void terminal_scroll(i32 amount) {
    terminal_set_scroll(scroll_position + amount);
}

void terminal_clear(void) {
    memset(active_line.text, '\0', (MAX_LINE_LEN + 1) * sizeof(char));
    active_line.len = 0;
    active_line.cursor_pos = 0;

    memset(closed_rows, 0, MAX_CLOSED_ROWS * sizeof(struct row));
    closed_rows_count = 0;

    scroll_position = 0;
}

void terminal_write(const char *text, bool is_error) {
    bool should_write_err_flag = is_error;
    bool is_last_char = false;
    for(i32 i = 0; !is_last_char; i++) {
        if(charqueue_is_full(output_buffer))
            break;

        char c;
        if(should_write_err_flag) {
            should_write_err_flag = false;
            i--;

            c = '\x0b';
        } else {
            c = text[i];
        }

        if(c == '\0') {
            c = '\n';
            is_last_char = true;
        } else if(c == '\n') {
            if(is_error)
                should_write_err_flag = true;
        }

        charqueue_enqueue(output_buffer, c);
    }
}

static bool is_empty(char *str) {
    for(u32 i = 0; str[i] != '\0'; i++) {
        if(str[i] != ' ')
            return false;
    }
    return true;
}

static void terminal_execute(void) {
    // save active line into command history
    if(!is_empty(active_line.text)) {
        circulararray_add(command_history, active_line.text, free);
        history_index = -1;
    }

    // parse command and arguments
    u32 splits_count = 0;
    // to avoid errors, just allocate MAX_LINE_LEN pointers
    char **splits = malloc(MAX_LINE_LEN * sizeof(const char *));

    u32 writing_index = 0;
    for(u32 i = 0; i < active_line.len; i++) {
        if(active_line.text[i] == ' ') {
            if(writing_index != 0)
                writing_index = 0;
        } else {
            if(writing_index == 0) {
                splits[splits_count] = calloc((MAX_LINE_LEN + 1), sizeof(char));
                splits_count++;
            }

            splits[splits_count - 1][writing_index] = active_line.text[i];
            writing_index++;
        }
    }

    if(splits_count == 0)
        commands_execute("", 0, NULL);
    else
        commands_execute(splits[0], splits_count - 1, splits + 1);

    // free memory
    for(u32 i = 0; i < splits_count; i++)
        free(splits[i]);
    free(splits);
}

static void allocate_active_line(void) {
    active_line.text = malloc((MAX_LINE_LEN + 1) * sizeof(char));
    active_line.text[0] = '\0';

    active_line.len = 0;
    active_line.cursor_pos = 0;
    active_line.type = LINE_TYPE_NORMAL;
}
