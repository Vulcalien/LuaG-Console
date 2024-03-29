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
#include "shell-commands.h"

#include "terminal.h"
#include "lua-engine.h"
#include "cartridge.h"
#include "archive-util.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static char *editor_folder = NULL;

static inline bool exists(char *filename) {
    struct stat st;
    return !stat(filename, &st);
}

static int check_is_developer(void) {
    if(!dev_mode) {
        terminal_write(
            "Error:\n"
            "only developers can\n"
            "use this command",
            true
        );
        return -1;
    }
    return 0;
}

#define CMD(name) static void name(u32 argc, char **argv)
#define CALL(command) command(argc, argv)
#define TEST(command) !strcmp(cmd, command)

CMD(cmd_run) {
    if(argc == 0) {
        if(dev_mode) {
            // check if USERDATA_FOLDER exists before running
            if(!exists(USERDATA_FOLDER)) {
                terminal_write(
                    "Error:\n"
                    "'" USERDATA_FOLDER "'\n"
                    "does not exist;\n"
                    "run 'setup' to generate\n"
                    "an empty project",
                    true
                );
                return;
            }

            game_folder = USERDATA_FOLDER;
            engine_load(false);
        } else {
            terminal_write(
                "Error: missing argument\n"
                "run [cartridge-name]",
                true
            );
        }
    } else {
        bool cartridge_found = false;

        for(u32 i = 0; i < 2; i++) {
            char *filename;

            if(i == 0) {
                filename = argv[0];
            } else {
                filename = malloc(PATH_MAX * sizeof(char));
                snprintf(filename, PATH_MAX, "%s.luag", argv[0]);
            }

            if(exists(filename)) {
                cartridge_found = true;

                game_folder = cartridge_extract(filename);
                if(game_folder) {
                    engine_load(false);
                } else {
                    terminal_write(
                        "Error:\n"
                        "could not extract\n"
                        "cartridge",
                        true
                    );
                }
            }

            if(i != 0)
                free(filename);

            if(cartridge_found)
                return;
        }

        char error_msg[4096];
        snprintf(
            error_msg, sizeof(error_msg) / sizeof(char),
            "Error:\n"
            "cartridge not found\n"
            "'%s'",
            argv[0]
        );
        terminal_write(error_msg, true);
    }
}

CMD(cmd_edit) {
    if(check_is_developer())
        return;

    if(!editor_folder) {
        editor_folder = malloc(PATH_MAX * sizeof(char));
        snprintf(editor_folder, PATH_MAX, "%s/editor", res_folder);
    }

    // check if USERDATA_FOLDER exists before running
    if(!exists(USERDATA_FOLDER)) {
        terminal_write(
            "Error:\n"
            "'" USERDATA_FOLDER "'\n"
            "does not exist;\n"
            "run 'setup' to generate\n"
            "an empty project",
            true
        );
        return;
    }

    game_folder = editor_folder;
    engine_load(true);
}

CMD(cmd_pack) {
    if(check_is_developer())
        return;

    if(argc == 0) {
        terminal_write(
            "Error: missing argument\n"
            "pack [cartridge-name]",
            true
        );
    } else {
        // check if USERDATA_FOLDER exists before trying to pack it
        if(!exists(USERDATA_FOLDER)) {
            terminal_write(
                "Error:\n"
                "'" USERDATA_FOLDER "'\n"
                "does not exist",
                true
            );
            return;
        }

        // check if 'cartridge-info' exists and if not then create it
        char *cartridge_info_filename = USERDATA_FOLDER "/cartridge-info";

        if(!exists(cartridge_info_filename)) {
            FILE *file = fopen(cartridge_info_filename, "w");
            fprintf(
                file, "library-version=%u.%u",
                CARTRIDGE_DEFAULT_MAJOR_V, CARTRIDGE_DEFAULT_MINOR_V
            );
            fclose(file);
        }

        // pack cartridge
        char filename[PATH_MAX];
        snprintf(filename, PATH_MAX, "%s.luag", argv[0]);

        if(archiveutil_pack(filename, USERDATA_FOLDER)) {
            terminal_write(
                "Error:\n"
                "could not create\n"
                "cartridge file",
                true
            );
        }
    }
}

CMD(cmd_unpack) {
    if(check_is_developer())
        return;

    if(argc == 0) {
        terminal_write(
            "Error: missing argument\n"
            "unpack [cartridge-name]",
            true
        );
    } else {
        if(exists(USERDATA_FOLDER)) {
            terminal_write(
                "Error:\n"
                "'" USERDATA_FOLDER "'\n"
                "already exists",
                true
            );
            return;
        }

        char filename[PATH_MAX];
        snprintf(filename, PATH_MAX, "%s.luag", argv[0]);

        if(archiveutil_extract(filename, USERDATA_FOLDER)) {
            terminal_write(
                "Error:\n"
                "could not unpack\n"
                "cartridge file",
                true
            );
        }
    }
}

CMD(cmd_setup) {
    if(check_is_developer())
        return;

    if(exists(USERDATA_FOLDER)) {
        terminal_write(
            "Error:\n"
            "'" USERDATA_FOLDER "'\n"
            "already exists",
            true
        );
        return;
    }

    char template_file[PATH_MAX];
    snprintf(
        template_file, PATH_MAX,
        "%s/template.luag", res_folder
    );

    if(archiveutil_extract(template_file, USERDATA_FOLDER)) {
        terminal_write(
            "Error:\n"
            "could not extract\n"
            "cartridge template",
            true
        );
    }
}

CMD(cmd_clear) {
    terminal_clear();
}

CMD(cmd_ver) {
    terminal_write(LUAG_VERSION, false);
    terminal_write(COPYRIGHT_NOTICE, false);
    terminal_write("This is Free software", false);
}

static void print_help_message(const char *cmd, const char *message) {
    char name[16] = { 0 };
    char str[32] = { 0 };
    snprintf(name, sizeof(name), "%s:", cmd);
    snprintf(str, sizeof(str), "%-7s %s", name, message);

    terminal_write(str, false);
}

CMD(cmd_help) {
    const char *list[][2] = {
        { "run",    "run game"          },
        { "edit",   "open editor"       },
        { "pack",   "create cartridge"  },
        { "unpack", "extract cartridge" },
        { "setup",  "create game files" },
        { "clear",  "clear shell"       },
        { "ver",    "print version"     },
        { "help",   "print this list"   },
        { "mode",   "change mode"       },
        { "files",  "open game folder"  },
        { "log",    "open log file"     },
        { NULL,     NULL                }
    };

    if(argc > 0) {
        char *cmd = argv[0];

        bool found = false;
        for(u32 i = 0; list[i][0] != NULL; i++) {
            if(!strcmp(cmd, list[i][0])) {
                found = true;
                print_help_message(list[i][0], list[i][1]);
            }
        }

        if(!found)
            terminal_write("Error: help message not\nfound", true);
    } else {
        for(u32 i = 0; list[i][0] != NULL; i++)
            print_help_message(list[i][0], list[i][1]);
    }
}

CMD(cmd_mode) {
    if(argc == 0) {
        terminal_write("current mode:", false);
        if(dev_mode)
            terminal_write("developer", false);
        else
            terminal_write("user", false);
    } else {
        char *mode = argv[0];

        // make mode lowercase
        for(u32 i = 0; mode[i] != 0; i++)
            mode[i] = tolower(mode[i]);

        if(!strcmp(mode, "d") || !strcmp(mode, "developer")) {
            dev_mode = true;
            terminal_write(
                "switching to\n"
                "developer mode",
                false
            );
        } else if(!strcmp(mode, "u") || !strcmp(mode, "user")) {
            dev_mode = false;
            terminal_write(
                "switching to\n"
                "user mode",
                false
            );
        } else {
            terminal_write(
                "Error\n"
                "unrecognized mode\n"
                "try 'd' or 'u'",
                true
            );
        }
    }
}

CMD(cmd_files) {
    if(check_is_developer())
        return;

    #ifdef __unix__
        system("xdg-open " USERDATA_FOLDER);
    #elif _WIN32
        system("explorer " USERDATA_FOLDER);
    #endif
}

CMD(cmd_log) {
}

CMD(cmd_exit) {
    should_quit = true;
}

int commands_init(void) {
    return 0;
}

void commands_destroy(void) {
    if(editor_folder)
        free(editor_folder);
}

void commands_execute(char *cmd, u32 argc, char **argv) {
    // ignore empty lines
    if(cmd[0] == '\0')
        return;

    // make cmd lowercase
    for(u32 i = 0; cmd[i] != '\0'; i++)
        cmd[i] = tolower(cmd[i]);

    if(TEST("run"))
        CALL(cmd_run);
    else if(TEST("edit") || TEST("editor"))
        CALL(cmd_edit);
    else if(TEST("pack"))
        CALL(cmd_pack);
    else if(TEST("unpack"))
        CALL(cmd_unpack);
    else if(TEST("setup"))
        CALL(cmd_setup);
    else if(TEST("clear") || TEST("cls"))
        CALL(cmd_clear);
    else if(TEST("ver") || TEST("version"))
        CALL(cmd_ver);
    else if(TEST("help"))
        CALL(cmd_help);
    else if(TEST("mode"))
        CALL(cmd_mode);
    else if(TEST("files"))
        CALL(cmd_files);
    else if(TEST("log"))
        CALL(cmd_log);
    else if(TEST("exit"))
        CALL(cmd_exit);
    else
        terminal_write("unknown command", false);

    terminal_write("", false);
}
