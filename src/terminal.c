/* Copyright 2022 Vulcalien
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

#include <string.h>

#include "display.h"
#include "shell-commands.h"

// colors
#define TERM_COLOR_NORMAL (0xffffff)
#define TERM_COLOR_ERROR  (0xff0000)
#define TERM_COLOR_INPUT  (0x00ff00)
#define CURSOR_COLOR      (0xff0000)

#define ROWS_IN_DISPLAY ((DISPLAY_HEIGHT - 2) / (CHAR_HEIGHT + LINE_SPACING))
#define CHARS_IN_ROW    ((DISPLAY_WIDTH - 2) / (CHAR_WIDTH + LETTER_SPACING))

#define ANIMATION_DELAY (25)

#define MAX_LINE_LEN (127)
#define MAX_CLOSED_ROWS (2048)

#define COMMAND_HISTORY_SIZE (1024)
#define MAX_BUFFERED_CHARS (4096)

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

static u32 scroll_position = 0;
static u32 cursor_animation_ticks = 0;

// closed rows
static struct row *closed_rows;
static u32 closed_rows_count = 0;

// command history
static char **command_history;
static u32 used_command_history = 0;

// BUFFERS
// user buffer
static char *user_buffer;
static u32 user_buffer_read_index = 0;
static u32 user_buffer_write_index = 0;

// output buffer
static char *output_buffer;
static u32 output_buffer_read_index = 0;
static u32 output_buffer_write_index = 0;

static void terminal_set_scroll(i32 scroll);

static void terminal_execute(void);

static void allocate_active_line(void);

int terminal_init(void) {
    allocate_active_line();

    closed_rows = calloc(MAX_CLOSED_ROWS, sizeof(struct row));
    command_history = malloc(COMMAND_HISTORY_SIZE * sizeof(char *));

    user_buffer   = malloc(MAX_BUFFERED_CHARS * sizeof(char));
    output_buffer = malloc(MAX_BUFFERED_CHARS * sizeof(char));

    return 0;
}

void terminal_destroy(void) {
    free(active_line.text);
    free(closed_rows);

    for(u32 i = 0; i < used_command_history; i++)
        free(command_history[i]);
    free(command_history);

    free(user_buffer);
    free(output_buffer);
}

static inline void check_closed_rows(void) {
    if(closed_rows_count == MAX_CLOSED_ROWS) {
        // overwrite the first half of the closed lines
        memcpy(
            closed_rows,
            closed_rows + MAX_CLOSED_ROWS / 2,
            MAX_CLOSED_ROWS / 2
        );
        closed_rows_count = MAX_CLOSED_ROWS / 2;
    }
}

void terminal_tick(void) {
    // TODO implement ctrl+backspace (= ctrl+w), ctrl+del, ctrl+u

    char c;

    if(output_buffer_read_index != output_buffer_write_index) {
        // output buffer is not empty
        c = output_buffer[output_buffer_read_index];
        output_buffer_read_index++;
        output_buffer_read_index %= MAX_BUFFERED_CHARS;

        if(active_line.type == LINE_TYPE_INPUT)
            active_line.type = LINE_TYPE_NORMAL;
    } else if(user_buffer_read_index != user_buffer_write_index) {
        // user buffer is not empty
        c = user_buffer[user_buffer_read_index];
        user_buffer_read_index++;
        user_buffer_read_index %= MAX_BUFFERED_CHARS;

        active_line.type = LINE_TYPE_INPUT;
    } else {
        // all buffers are empty
        cursor_animation_ticks++;
        return;
    }

    cursor_animation_ticks = 0;

    if(c == '\n') {
        // split and save the active line into closed_rows
        struct row *current_row;
        for(u32 i = 0; i <= active_line.len; i++) {
            // if this then it is time to switch row
            if(i % CHARS_IN_ROW == 0 || i == active_line.len) {
                // close the last row
                if(i > 0 || active_line.len == 0) {
                    closed_rows_count++;
                    check_closed_rows();
                }

                // grab the next row
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
            active_line.len--;
            active_line.cursor_pos--;
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
        // TODO history ...
    } else if(c == '\x12') {
        // left key
        if(active_line.cursor_pos != 0)
            active_line.cursor_pos--;
    } else if(c == '\x13') {
        // down key
        // TODO history ...
    } else if(c == '\x14') {
        // right key
        if(active_line.cursor_pos != active_line.len)
            active_line.cursor_pos++;
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
            active_line.len++;
            active_line.cursor_pos++;
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
            1, 1 + (CHAR_HEIGHT + LINE_SPACING) * rendered_lines
        );

        rendered_lines++;
        if(rendered_lines >= ROWS_IN_DISPLAY) // TODO test this
            break;
    }

    if(rendered_lines < ROWS_IN_DISPLAY) { // TODO test this
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
            1, 1 + (CHAR_HEIGHT + LINE_SPACING) * rendered_lines
        );

        if(
            (active_line.type == LINE_TYPE_INPUT || cursor_animation_ticks != 0)
            && cursor_animation_ticks / ANIMATION_DELAY % 2 == 0
        ) {
            display_write(
                "_", CURSOR_COLOR,
                1 + (CHAR_WIDTH + LETTER_SPACING) * active_line.cursor_pos,
                1 + (CHAR_HEIGHT + LINE_SPACING) * rendered_lines
            );
        }
    }
}

void terminal_receive_input(const char *c) {
    for(u32 i = 0; c[i] != '\0'; i++) {
        // the buffer is full
        if(user_buffer_write_index + 1 == user_buffer_read_index)
            break;

        user_buffer[user_buffer_write_index] = c[i];
        user_buffer_write_index++;
        user_buffer_write_index %= MAX_BUFFERED_CHARS;
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
        // the buffer is full
        if(output_buffer_write_index + 1 == output_buffer_read_index)
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

        output_buffer[output_buffer_write_index] = c;

        output_buffer_write_index++;
        output_buffer_write_index %= MAX_BUFFERED_CHARS;
    }
}

static inline void check_command_history(void) {
    if(used_command_history == COMMAND_HISTORY_SIZE) {
        // free the oldest half of commands
        for(u32 i = 0; i < COMMAND_HISTORY_SIZE / 2; i++)
            free(command_history[i]);

        // overwrite the first half
        memcpy(
            command_history,
            command_history + COMMAND_HISTORY_SIZE / 2,
            COMMAND_HISTORY_SIZE / 2
        );
        used_command_history = COMMAND_HISTORY_SIZE / 2;
    }
}

static void terminal_execute(void) {
    check_command_history();
    command_history[used_command_history] = active_line.text;
    used_command_history++;

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
        execute_command("", 0, NULL);
    else
        execute_command(splits[0], splits_count - 1, splits + 1);

    // free memory
    for(u32 i = 0; i < splits_count; i++)
        free(splits[i]);
    free(splits);
}

static void allocate_active_line(void) {
    active_line.text = calloc((MAX_LINE_LEN + 1), sizeof(char));
    active_line.len = 0;
    active_line.cursor_pos = 0;
    active_line.type = LINE_TYPE_NORMAL;
}
