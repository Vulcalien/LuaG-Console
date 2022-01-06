/* Copyright 2022 Vulcalien
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 only.
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

#define MAX_LINE_LEN (127)

#define CHARS_IN_ROW ((DISPLAY_WIDTH - 2) / (CHAR_WIDTH + LINE_SPACING))
#define MAX_CLOSED_ROWS (2048)

#define COMMAND_HISTORY_SIZE (1024)

#define MAX_BUFFERED_CHARS (2048)

static struct {
    char *text;
    u32 len;
    u32 cursor_pos;
} active_line;

struct row {
    char text[CHARS_IN_ROW + 1];
    u32 color;
};

struct buffered_char {
    char c;
    bool is_user_input;
};

static struct row *closed_rows;
static u32 closed_rows_count = 0;

static char **command_history;
static u32 used_command_history = 0;

static struct buffered_char *char_buffer;
static u32 char_buffer_read_index = 0;
static u32 char_buffer_write_index = 0;

static void terminal_execute(void);

static void allocate_active_line(void);

int terminal_init(void) {
    allocate_active_line();

    closed_rows = calloc(MAX_CLOSED_ROWS, sizeof(struct row));
    command_history = malloc(COMMAND_HISTORY_SIZE * sizeof(char *));
    char_buffer = malloc(MAX_BUFFERED_CHARS * sizeof(struct buffered_char));

    return 0;
}

int terminal_destroy(void) {
    free(active_line.text);
    free(closed_rows);

    for(u32 i = 0; i < used_command_history; i++)
        free(command_history[i]);
    free(command_history);

    free(char_buffer);

    return 0;
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

    // the buffer is empty
    if(char_buffer_read_index == char_buffer_write_index)
        return;

    struct buffered_char buf_char = char_buffer[char_buffer_read_index];
    char_buffer_read_index++;
    char_buffer_read_index %= MAX_BUFFERED_CHARS;

    char c = buf_char.c;

    if(c == '\n' || c == '\x0b') {
        // split and save the active line into closed_rows
        struct row *current_row;
        for(u32 i = 0; i <= active_line.len; i++) {
            if(i % CHARS_IN_ROW == 0 || i == active_line.len) {
                // close the row
                if(i > 0 || active_line.len == 0) {
                    closed_rows_count++;
                    check_closed_rows();
                }

                // grab the next row
                if(i != active_line.len) {
                    current_row = &closed_rows[closed_rows_count];

                    if(buf_char.is_user_input)
                        current_row->color = TERM_COLOR_INPUT;
                    else if(c == '\x0b')
                        current_row->color = TERM_COLOR_ERROR;
                    else
                        current_row->color = TERM_COLOR_NORMAL;
                }
            }
            // put a char into the row we are writing in
            if(active_line.len > 0)
                current_row->text[i % CHARS_IN_ROW] = active_line.text[i];
        }

        if(buf_char.is_user_input)
            terminal_execute();

        allocate_active_line();
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

    luag_ask_refresh();
}

void terminal_render(void) {
    display_clear(0x000000);

    display_write(active_line.text, 0xffffff, 1, 1);
    display_write("_", 0xff0000, 1 + (CHAR_WIDTH + LETTER_SPACING) * active_line.cursor_pos, 1);

    for(u32 i = 0; i < closed_rows_count; i++) {
        display_write(
            closed_rows[i].text, closed_rows[i].color,
            1, 1 + 12 * (i + 1)
        );
    }
}

void terminal_receive_input(const char *c) {
    // the buffer is full
    if(char_buffer_write_index + 1 == char_buffer_read_index)
        return;

    for(u32 i = 0; c[i] != '\0'; i++) {
        char_buffer[char_buffer_write_index] = (struct buffered_char) {
            .c = c[i],
            .is_user_input = true
        };
        char_buffer_write_index++;
        char_buffer_write_index %= MAX_BUFFERED_CHARS;
    }
}

void terminal_clear(void) {
    memset(active_line.text, '\0', (MAX_LINE_LEN + 1) * sizeof(char));
    active_line.len = 0;
    active_line.cursor_pos = 0;

    memset(closed_rows, 0, MAX_CLOSED_ROWS * sizeof(struct row));
    closed_rows_count = 0;
}

void terminal_write(const char *text, bool is_error) {
    for(u32 i = 0; text[i] != '\0'; i++) {
        char_buffer[char_buffer_write_index] = (struct buffered_char) {
            .is_user_input = false
        };

        if(text[i] == '\n' && is_error)
            char_buffer[char_buffer_write_index].c = '\x0b';
        else
            char_buffer[char_buffer_write_index].c = text[i];

        char_buffer_write_index++;
        char_buffer_write_index %= MAX_BUFFERED_CHARS;
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
}
