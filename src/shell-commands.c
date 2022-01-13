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

#include <ctype.h>
#include <string.h>

#define CMD(name) void name(u32 argc, char **argv)
#define CALL(command) command(argc, argv)
#define TEST(command) !strcmp(cmd, command)

static CMD(cmd_run) {
    engine_load();
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
}

static CMD(cmd_files) {
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
