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
#include "luag-console.h"

#include <lua5.4/lua.h>
#include <lua5.4/lauxlib.h>

#include "display.h"

// DEBUG
#include <stdio.h>

#define F(name) static int name(lua_State *L)

F(clear) {
    lua_Integer color = luaL_checkinteger(L, 1);

    /*display_clear(color);*/
    return 0;
}

static void setf(lua_State *L, int (*func)(lua_State *L), const char *name) {
    lua_pushcfunction(L, func);
    lua_setglobal(L, name);
}

int luag_lib_load(lua_State *L) {
    // TODO add variables
    // TODO add more functions and order them

    setf(L, clear, "clear");

    return 0;
}

int luag_lib_destroy(void) {
    puts("Bye shared library!");

    return 0;
}
