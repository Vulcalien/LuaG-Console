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
#include "lua-engine.h"

#include "terminal.h"

#include <stdio.h>

#include <lua5.4/lua.h>
#include <lua5.4/lualib.h>
#include <lua5.4/lauxlib.h>

#define BUFFER_SIZE (4 * 1024)

void engine_load(void) {
    lua_State *L = luaL_newstate();

    // load libraries

    luaopen_base(L);
    // coroutine
    // package
    luaopen_string(L);
    // UTF-8
    luaopen_table(L);
    luaopen_math(L);
    // io
    // os
    // debug

    // load and run main.lua
    if(luaL_dofile(L, "scripts/main.lua")) {
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        lua_pop(L, 1); // TODO is this lua_pop correct?

        // TODO print this to a log file and
        // write "See log file" in terminal
    }
}

void engine_tick(void) {
}

void engine_render(void) {
}
