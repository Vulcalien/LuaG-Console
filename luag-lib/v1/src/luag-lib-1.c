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

#include <limits.h>

#include <lua5.4/lua.h>
#include <lua5.4/lauxlib.h>

#include "display.h"

#define F(name) static int name(lua_State *L)

// generic
F(loadscript) {
    // TODO make sure this can't escape the scripts
    // folder using '..' or other ways
    const char *filename = luaL_checkstring(L, 1);

    char *full_path = malloc(PATH_MAX * sizeof(char));
    snprintf(
        full_path, PATH_MAX,
        "%s/scripts/%s", game_folder, filename
    );

    if(luaL_dofile(L, full_path))
        lua_error(L);

    free(full_path);
    return 0;
}

F(luag_log) {
    return 0;
}

// keys
F(key) {
    return 1;
}

F(key_pressed) {
    return 1;
}

F(key_released) {
    return 1;
}

// sound
F(sfx) {
    return 0;
}

F(sfx_loop) {
    return 0;
}

F(sfx_stop) {
    return 0;
}

// screen
F(settransparent) {
    return 0;
}

F(clear) {
    lua_Integer color = luaL_checkinteger(L, 1);
    display_clear(color);
    return 0;
}

F(pix) {
    lua_Integer x     = luaL_checkinteger(L, 1);
    lua_Integer y     = luaL_checkinteger(L, 2);
    lua_Integer color = luaL_checkinteger(L, 3);

    lua_Integer w = lua_isnil(L, 4) ? 1 : luaL_checkinteger(L, 4);
    lua_Integer h = lua_isnil(L, 5) ? 1 : luaL_checkinteger(L, 5);

    display_fill(x, y, w, h, color);
    return 0;
}

F(write) {
    const char *text  = luaL_checkstring(L, 1);
    lua_Integer color = luaL_checkinteger(L, 2);
    lua_Integer x     = luaL_checkinteger(L, 3);
    lua_Integer y     = luaL_checkinteger(L, 4);

    display_write(text, color, x, y);
    return 0;
}

F(spr) {
    lua_Integer id = luaL_checkinteger(L, 1);
    lua_Integer x  = luaL_checkinteger(L, 2);
    lua_Integer y  = luaL_checkinteger(L, 3);

    lua_Integer scale = lua_isnil(L, 4) ? 1 : luaL_checkinteger(L, 4);

    lua_Integer sw = lua_isnil(L, 5) ? 1 : luaL_checkinteger(L, 5);
    lua_Integer sh = lua_isnil(L, 6) ? 1 : luaL_checkinteger(L, 6);

    lua_Integer rot = lua_isnil(L, 7) ? 0 : luaL_checkinteger(L, 7);

    bool h_flip = lua_isnil(L, 8) ? false : lua_toboolean(L, 8);
    bool v_flip = lua_isnil(L, 9) ? false : lua_toboolean(L, 9);

    bool err = true;
    if(id < 0 || id >= 256)
        lua_pushliteral(L, "bad argument: id");
    else if(scale <= 0)
        lua_pushliteral(L, "bad argument: scale");
    else if(sw <= 0 || (id % 16) + sw > 16)
        lua_pushliteral(L, "bad argument: sw");
    else if(sh <= 0 || (id / 16) + sh > 16)
        lua_pushliteral(L, "bad argument: sh");
    else
        err = false;

    if(err) {
        lua_error(L);
    } else {
        display_draw_from_atlas(
            id, x, y,
            scale, sw, sh,
            rot, h_flip, v_flip
        );
    }
    return 0;
}

// map
F(get_tile) {
    return 1;
}

F(set_tile) {
    return 1;
}

F(maprender) {
    return 0;
}

static void setf(lua_State *L, int (*func)(lua_State *L), const char *name) {
    lua_pushcfunction(L, func);
    lua_setglobal(L, name);
}

int luag_lib_load(lua_State *L) {
    // VARIABLES
    lua_pushinteger(L, DISPLAY_WIDTH);
    lua_setglobal(L, "scr_w");

    lua_pushinteger(L, DISPLAY_HEIGHT);
    lua_setglobal(L, "scr_h");

    lua_pushinteger(L, CHAR_WIDTH);
    lua_setglobal(L, "font_w");

    lua_pushinteger(L, CHAR_HEIGHT);
    lua_setglobal(L, "font_h");

    // TODO map_w and map_h variables

    // FUNCTIONS
    // generic
    setf(L, loadscript, "loadscript");
    setf(L, luag_log, "log");

    // keys
    setf(L, key, "key");
    setf(L, key, "key_down");
    setf(L, key_pressed, "key_pressed");
    setf(L, key_released, "key_released");

    // sound
    setf(L, sfx, "sfx");
    setf(L, sfx, "sfx_play");
    setf(L, sfx_loop, "sfx_loop");
    setf(L, sfx_stop, "sfx_stop");

    // screen
    setf(L, settransparent, "settransparent");
    setf(L, clear, "clear");
    setf(L, pix, "pix");
    setf(L, write, "write");
    setf(L, spr, "spr");

    // map
    setf(L, get_tile, "get_tile");
    setf(L, set_tile, "set_tile");
    setf(L, maprender, "maprender");

    return 0;
}

int luag_lib_destroy(void) {
    return 0;
}
