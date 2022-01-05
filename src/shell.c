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
#include "shell.h"

#include <string.h>

#include "display.h"
#include "shell-commands.h"

#define COMMAND_HISTORY_SIZE (1024)
static char **command_history;
static u32 used_command_history = 0;

#define MAX_LINE_LEN (127)

static struct {
    char *text;
    u32 len;
    u32 cursor_pos;
} active_line;

static void shell_execute(void);

static void allocate_active_line(void);

int shell_init(void) {
    command_history = malloc(COMMAND_HISTORY_SIZE * sizeof(const char *));

    allocate_active_line();

    return 0;
}

void shell_tick(void) {
}

void shell_render(void) {
    display_clear(0x000000);

    display_write(active_line.text, 0xffffff, 1, 1);
    display_write("_", 0xff0000, 1 + (CHAR_WIDTH + LETTER_SPACING) * active_line.cursor_pos, 1);
}

// TODO
void shell_write(const char *text, u32 color) {
}

void shell_receive_input(const char *c) {
    for(u32 i = 0; c[i] != '\0'; i++) {
        if(c[i] == '\n') {
            shell_execute();
        } else if(c[i] == '\b') {
            if(active_line.cursor_pos == 0)
                continue;

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
        } else if(c[i] == '\x7f') {
            if(active_line.cursor_pos == active_line.len)
                continue;

            if(active_line.cursor_pos < active_line.len - 1) {
                memmove(
                    active_line.text + active_line.cursor_pos,
                    active_line.text + active_line.cursor_pos + 1,
                    active_line.len - active_line.cursor_pos
                );
            }
            active_line.len--;
            active_line.text[active_line.len] = '\0';
        } else if(c[i] == '\x11') {
            // this represents the up key
            // TODO history ...
        } else if(c[i] == '\x12') {
            // left key
            if(active_line.cursor_pos != 0)
                active_line.cursor_pos--;
        } else if(c[i] == '\x13') {
            // down key
            // TODO history ...
        } else if(c[i] == '\x14') {
            // right key
            if(active_line.cursor_pos != active_line.len)
                active_line.cursor_pos++;
        } else if(c[i] >= ' ' && c[i] <= '~') {
            if(active_line.len == MAX_LINE_LEN)
                return;

            if(active_line.cursor_pos != active_line.len) {
                memmove(
                    active_line.text + active_line.cursor_pos + 1,
                    active_line.text + active_line.cursor_pos,
                    active_line.len - active_line.cursor_pos
                );
            }
            active_line.text[active_line.cursor_pos] = c[i];
            active_line.len++;
            active_line.cursor_pos++;
        }
    }

    luag_ask_refresh();
}

static void shell_execute(void) {
    if(used_command_history == COMMAND_HISTORY_SIZE) {
        // delete the oldest half of commands

        for(u32 i = 0; i < COMMAND_HISTORY_SIZE; i++) {
            free(command_history[i]);
        }

        // overwrite the first half
        memcpy(
            command_history,
            command_history + COMMAND_HISTORY_SIZE / 2,
            COMMAND_HISTORY_SIZE / 2
        );
        used_command_history = COMMAND_HISTORY_SIZE / 2;
    }
    command_history[used_command_history] = active_line.text;

    // parse command and arguments
    u32 splits_count = 0;
    // to avoid errors, just allocate MAX_LINE_LEN pointers
    char **splits = malloc(MAX_LINE_LEN * sizeof(const char *));

    u32 writing_index = 0;
    for(u32 i = 0; i < active_line.len; i++) {
        if(active_line.text[i] == ' ') {
            if(writing_index != 0) {
                splits[splits_count][writing_index] = '\0';
                splits_count++;
                writing_index = 0;
            }
        } else {
            if(writing_index == 0)
                splits[splits_count] = malloc(MAX_LINE_LEN * sizeof(char));

            splits[splits_count][writing_index] = active_line.text[i];
            writing_index++;
        }
    }

    // if the last word was not closed, close it
    if(writing_index != 0) {
        splits[splits_count][writing_index] = '\0';
        splits_count++;
    }

    if(splits_count > 0)
        execute_command(splits[0], splits_count - 1, splits + 1);

    // free memory
    for(u32 i = 0; i < splits_count; i++)
        free(splits[i]);
    free(splits);

    allocate_active_line();
}

static void allocate_active_line(void) {
    active_line.text = calloc((MAX_LINE_LEN + 1), sizeof(char));
    active_line.len = 0;
    active_line.cursor_pos = 0;
}
