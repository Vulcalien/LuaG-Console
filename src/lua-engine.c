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

static lua_State *L = NULL;

// returns nonzero if the engine was stopped
static int check_error(lua_State *L, int status) {
    if(status != LUA_OK) {
        const char *error_msg = lua_tostring(L, -1);

        fprintf(stderr, "%s\n", error_msg);

        lua_pop(L, 1);

        engine_stop();
        return 1;
    }
    return 0;
}

void engine_load(void) {
    if(L != NULL) {
        fputs("Error: lua_State 'L' is not NULL on engine_load\n", stderr);
        return;
    }

    L = luaL_newstate();

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

    int status;

    // load and run main.lua
    status = luaL_dofile(L, "scripts/main.lua");
    if(check_error(L, status))
        return;

    // run "init" function
    lua_getglobal(L, "init");
    if(!lua_isfunction(L, -1)) {
        fputs("Error: 'main.lua' must contain a function 'init()'\n", stderr);

        engine_stop();
        return;
    }

    status = lua_pcall(L, 0, 0, 0);
    if(check_error(L, status))
        return;
}

void engine_stop(void) {
    lua_close(L);
    L = NULL;
}

void engine_tick(void) {
    lua_getglobal(L, "tick");
    if(!lua_isfunction(L, -1)) {
        fputs("Error: a function 'tick()' must be defined\n", stderr);

        engine_stop();
        return;
    }

    int status = lua_pcall(L, 0, 0, 0);
    if(check_error(L, status))
        return;
}
