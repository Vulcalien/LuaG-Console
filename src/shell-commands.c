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
#include "shell-commands.h"

#include "terminal.h"
#include "lua-engine.h"
#include "cartridge.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

static int check_is_developer(void) {
    if(!dev_mode) {
        terminal_write(
            "Error:\n"
            "only developers can\n"
            "use this command",
            false
        );
        return -1;
    }
    return 0;
}

#define CMD(name) void name(u32 argc, char **argv)
#define CALL(command) command(argc, argv)
#define TEST(command) !strcmp(cmd, command)

static CMD(cmd_run) {
    if(argc == 0) {
        if(dev_mode) {
            game_folder = "console-userdata";
            engine_load();
        } else {
            terminal_write(
                "Error:\n"
                "insert cartridge name\n",
                true
            );
        }
    } else {
        char *filename = malloc(PATH_MAX * sizeof(char));
        snprintf(
            filename, PATH_MAX,
            "%s.luag", argv[0]
        );
        game_folder = cartridge_extract(filename);

        if(game_folder) {
            engine_load();
        } else {
            char *error_msg = malloc(128 * sizeof(char));
            snprintf(
                error_msg, 128,
                "Error:\n"
                "'%s'\n"
                "cartridge not found\n",
                filename
            );

            terminal_write(error_msg, true);
            free(error_msg);
        }

        free(filename);
    }
}

static CMD(cmd_edit) {
}

static CMD(cmd_pack) {
}

static CMD(cmd_setup) {
}

static CMD(cmd_cls) {
    terminal_clear();
}

static CMD(cmd_ver) {
    terminal_write(LUAG_VERSION "\n", false);
    terminal_write(COPYRIGHT_NOTICE "\n", false);
    terminal_write("This is Free software\n", false);
}

static CMD(cmd_help) {
    if(argc > 0) {

    } else {
        terminal_write("run: runs game\n", false);
        terminal_write("edit: opens editor\n", false);
        terminal_write("pack: creates cartridge\n", false);
        terminal_write("setup: creates game files\n", false);
        terminal_write("cls: clears shell\n", false);
        terminal_write("ver: prints version\n", false);
        terminal_write("help: prints this list\n", false);
        terminal_write("mode: changes console mode\n", false);
        terminal_write("files: opens game folder\n", false);
        terminal_write("log: opens log file\n", false);
        terminal_write("\n", false);
    }
}

static CMD(cmd_mode) {
    if(argc == 0) {
        terminal_write("current mode:\n", false);
        if(dev_mode)
            terminal_write("developer\n", false);
        else
            terminal_write("user\n", false);
    } else {
        const char *mode = argv[0];
        if(!strcmp(mode, "d") || !strcmp(mode, "developer")) {
            dev_mode = true;
            terminal_write(
                "switching to\n"
                "developer mode\n",
                false
            );
        } else if(!strcmp(mode, "u") || !strcmp(mode, "user")) {
            dev_mode = false;
            terminal_write(
                "switching to\n"
                "user mode\n",
                false
            );
        } else {
            terminal_write(
                "Error\n"
                "unrecognized mode\n"
                "try 'd' or 'u'\n",
                true
            );
        }
    }
}

static CMD(cmd_files) {
    if(check_is_developer())
        return;

    #ifdef __unix__
        system("xdg-open console-userdata");
    #elif _WIN32
        system("explorer console-userdata");
    #endif
}

static CMD(cmd_log) {
}

static CMD(cmd_exit) {
    should_quit = true;
}

bool execute_command(char *cmd, u32 argc, char **argv) {
    // make cmd lowercase
    for(u32 i = 0; cmd[i] != '\0'; i++) {
        cmd[i] = tolower(cmd[i]);
    }

    if(TEST("run"))
        CALL(cmd_run);
    else if(TEST("edit") || TEST("editor"))
        CALL(cmd_edit);
    else if(TEST("pack"))
        CALL(cmd_pack);
    else if(TEST("setup"))
        CALL(cmd_setup);
    else if(TEST("cls") || TEST("clear"))
        CALL(cmd_cls);
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
        terminal_write("unknown command\n", false);

    terminal_write("\n", false);

    return false;
}
