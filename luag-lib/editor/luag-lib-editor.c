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
#include "luag-console.h"

#include <lua5.4/lua.h>
#include <lua5.4/lauxlib.h>

#define F(name) static int name(lua_State *L)

static void throw_lua_error(lua_State *L, char *msg_format, ...) {
    va_list args;
    va_start(args, msg_format);

    luaL_where(L, 1);
    lua_pushvfstring(L, msg_format, args);
    lua_concat(L, 2);
    lua_error(L);

    va_end(args);
}

int luag_lib_load(lua_State *L) {
    return 0;
}

int luag_lib_destroy(void) {
    return 0;
}
