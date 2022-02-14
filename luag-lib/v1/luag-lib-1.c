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

#include "display.h"
#include "map.h"
#include "input.h"
#include "sound.h"

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#include <lua5.4/lua.h>
#include <lua5.4/lauxlib.h>

#define F(name) static int name(lua_State *L)

#define prefix(pre, str) !strncmp(pre, str, strlen(pre))

static void throw_lua_error(lua_State *L, char *msg_format, ...) {
    va_list args;
    va_start(args, msg_format);

    luaL_where(L, 1);
    lua_pushvfstring(L, msg_format, args);
    lua_concat(L, 2);
    lua_error(L);

    va_end(args);
}

// generic
F(loadscript) {
    const char *script_filename = luaL_checkstring(L, 1);

    char *file_rel_path          = malloc(PATH_MAX * sizeof(char));
    char *file_abs_path_m        = malloc(PATH_MAX * sizeof(char));
    char *game_folder_abs_path_m = malloc(PATH_MAX * sizeof(char));

    snprintf(
        file_rel_path, PATH_MAX,
        "%s/scripts/%s", game_folder, script_filename
    );

    char *file_abs_path = realpath(file_rel_path, file_abs_path_m);
    if(!file_abs_path) {
        char *err_msg;
        if(errno == EACCES)
            err_msg = "cannot open %s: Permission denied";
        else if(errno == ENOENT)
            err_msg = "cannot open %s: No such file or directory";
        else
            err_msg = "cannot open %s";

        throw_lua_error(L, err_msg, script_filename);

        goto exit;
    }

    char *game_folder_abs_path = realpath(game_folder, game_folder_abs_path_m);
    if(!game_folder_abs_path) {
        fputs("Error: could not find realpath of cartridge folder\n", stderr);
        goto exit;
    }

    bool is_file_in_folder = prefix(game_folder_abs_path, file_abs_path);

    if(is_file_in_folder) {
        if(luaL_dofile(L, file_rel_path))
            lua_error(L);
    } else {
        throw_lua_error(
            L,
            "attempt to load a script outside of the cartridge folder"
        );
    }

    exit:
    free(file_rel_path);
    free(file_abs_path_m);
    free(game_folder_abs_path_m);
    return 0;
}

F(luag_log) {
    // TODO
    return 0;
}

// keys
F(key) {
    lua_Integer id = luaL_checkinteger(L, 1);

    if(id < 0 || id >= KEY_COUNT) {
        throw_lua_error(L, "bad argument: key '%d' does not exist", id);
        return 0;
    }

    lua_pushboolean(L, input_keys[id].is_down);
    return 1;
}

F(key_pressed) {
    lua_Integer id = luaL_checkinteger(L, 1);

    if(id < 0 || id >= KEY_COUNT) {
        throw_lua_error(L, "bad argument: key '%d' does not exist", id);
        return 0;
    }

    lua_pushboolean(L, input_keys[id].press_count);
    return 1;
}

F(key_released) {
    lua_Integer id = luaL_checkinteger(L, 1);

    if(id < 0 || id >= KEY_COUNT) {
        throw_lua_error(L, "bad argument: key '%d' does not exist", id);
        return 0;
    }

    lua_pushboolean(L, input_keys[id].release_count);
    return 1;
}

// mouse
F(mouse) {
    lua_Integer id = luaL_checkinteger(L, 1);

    if(id < 0 || id >= BTN_COUNT) {
        throw_lua_error(L, "bad argument: button '%d' does not exist", id);
        return 0;
    }

    lua_pushboolean(L, input_btns[id].is_down);
    return 1;
}

F(mouse_pressed) {
    lua_Integer id = luaL_checkinteger(L, 1);

    if(id < 0 || id >= BTN_COUNT) {
        throw_lua_error(L, "bad argument: button '%d' does not exist", id);
        return 0;
    }

    lua_pushboolean(L, input_btns[id].press_count);
    return 1;
}

F(mouse_released) {
    lua_Integer id = luaL_checkinteger(L, 1);

    if(id < 0 || id >= BTN_COUNT) {
        throw_lua_error(L, "bad argument: button '%d' does not exist", id);
        return 0;
    }

    lua_pushboolean(L, input_btns[id].release_count);
    return 1;
}

F(mouse_pos) {
    lua_pushinteger(L, input_mouse.x);
    lua_pushinteger(L, input_mouse.y);
    return 2;
}

F(scroll) {
    lua_pushinteger(L, input_mouse.scroll);
    return 1;
}

// sound
F(sfx) {
    const char *name = luaL_checkstring(L, 1);
    if(sound_play(name, 0))
        throw_lua_error(L, "bad argument: sound '%s' does not exist", name);
    return 0;
}

F(sfx_loop) {
    const char *name = luaL_checkstring(L, 1);
    if(sound_play(name, -1))
        throw_lua_error(L, "bad argument: sound '%s' does not exist", name);
    return 0;
}

F(sfx_stop) {
    const char *name = luaL_checkstring(L, 1);
    if(sound_stop(name))
        throw_lua_error(L, "bad argument: sound '%s' does not exist", name);
    return 0;
}

// screen
F(settransparent) {
    bool active_flag = !lua_isnoneornil(L, 1);
    lua_Integer color = active_flag ? luaL_checkinteger(L, 1) : 0x000000;

    display_atlas_set_color_key(color, active_flag);

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

    lua_Integer w = luaL_optinteger(L, 4, 1);
    lua_Integer h = luaL_optinteger(L, 5, 1);

    display_fill(x, y, w, h, color, 0xff);
    return 0;
}

F(write) {
    const char *text  = luaL_checkstring(L, 1);
    lua_Integer color = luaL_checkinteger(L, 2);
    lua_Integer x     = luaL_checkinteger(L, 3);
    lua_Integer y     = luaL_checkinteger(L, 4);

    display_write(text, color, 0xff, x, y);
    return 0;
}

F(spr) {
    lua_Integer id = luaL_checkinteger(L, 1);
    lua_Integer x  = luaL_checkinteger(L, 2);
    lua_Integer y  = luaL_checkinteger(L, 3);

    lua_Integer scale = luaL_optinteger(L, 4, 1);

    lua_Integer sw = luaL_optinteger(L, 5, 1);
    lua_Integer sh = luaL_optinteger(L, 6, 1);

    lua_Integer rot = luaL_optinteger(L, 7, 0);

    bool h_flip = luaL_opt(L, lua_toboolean, 8, false);
    bool v_flip = luaL_opt(L, lua_toboolean, 9, false);

    lua_Integer col_mod = luaL_optinteger(L, 10, 0xffffff);

    char *err_msg = NULL;
    if(id < 0 || id >= 256)
        err_msg = "bad argument: id";
    else if(sw <= 0 || (id % 16) + sw > 16)
        err_msg = "bad argument: sw";
    else if(sh <= 0 || (id / 16) + sh > 16)
        err_msg = "bad argument: sh";
    else if(scale <= 0)
        err_msg = "bad argument: scale";

    if(err_msg) {
        throw_lua_error(L, err_msg);
    } else {
        display_draw_from_atlas(
            NULL,
            id, x, y,
            scale, sw, sh,
            rot, h_flip, v_flip,
            0xff, col_mod
        );
    }
    return 0;
}

// map
F(get_tile) {
    lua_Integer x = luaL_checkinteger(L, 1);
    lua_Integer y = luaL_checkinteger(L, 2);

    char *err_msg = NULL;
    if(x < 0 || x >= map.width)
        err_msg = "bad argument: x";
    else if(y < 0 || y >= map.height)
        err_msg = "bad argument: y";

    if(err_msg) {
        throw_lua_error(L, err_msg);
        return 0;
    } else {
        u8 tile = map_get_tile(x, y);
        lua_pushinteger(L, tile);
        return 1;
    }
}

F(set_tile) {
    lua_Integer x  = luaL_checkinteger(L, 1);
    lua_Integer y  = luaL_checkinteger(L, 2);
    lua_Integer id = luaL_checkinteger(L, 3);

    char *err_msg = NULL;
    if(x < 0 || x >= map.width)
        err_msg = "bad argument: x";
    else if(y < 0 || y >= map.height)
        err_msg = "bad argument: y";
    else if(id < 0 || id >= 256)
        err_msg = "bad argument: id";

    if(err_msg) {
        throw_lua_error(L, err_msg);
    } else {
        map_set_tile(x, y, id);
    }
    return 0;
}

F(maprender) {
    lua_Integer scale = luaL_optinteger(L, 1, 1);

    lua_Integer xoff = luaL_optinteger(L, 2, 0);
    lua_Integer yoff = luaL_optinteger(L, 3, 0);

    char *err_msg = NULL;
    if(scale <= 0)
        err_msg = "bad argument: scale";

    if(err_msg) {
        throw_lua_error(L, err_msg);
    } else {
        const u32 tile_size = SPRITE_SIZE * scale;

        i32 xt0 = xoff / tile_size;
        if(xoff < 0) xt0--;

        i32 yt0 = yoff / tile_size;
        if(yoff < 0) yt0--;

        i32 xt1 = xt0 + (DISPLAY_WIDTH / tile_size) + 1;
        i32 yt1 = yt0 + (DISPLAY_HEIGHT / tile_size) + 1;

        // check boundaries
        if(xt0 < 0) xt0 = 0;
        if(yt0 < 0) yt0 = 0;

        if(xt1 > map.width) xt1 = map.width;
        if(yt1 > map.height) yt1 = map.height;

        for(u32 yt = yt0; yt < yt1; yt++) {
            for(u32 xt = xt0; xt < xt1; xt++) {
                u32 id = map_get_tile(xt, yt);
                display_draw_from_atlas(
                    NULL,
                    id, xt * tile_size - xoff, yt * tile_size - yoff,
                    scale, 1, 1,
                    0, false, false,
                    0xff, 0xffffff
                );
            }
        }
    }
    return 0;
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

    lua_pushinteger(L, map.width);
    lua_setglobal(L, "map_w");

    lua_pushinteger(L, map.height);
    lua_setglobal(L, "map_h");

    // FUNCTIONS
    // generic
    lua_register(L, "loadscript", loadscript);
    lua_register(L, "log", luag_log);

    // keys
    lua_register(L, "key", key);
    lua_register(L, "key_down", key);
    lua_register(L, "key_pressed", key_pressed);
    lua_register(L, "key_released", key_released);

    // mouse
    lua_register(L, "mouse", mouse);
    lua_register(L, "mouse_down", mouse);
    lua_register(L, "mouse_pressed", mouse_pressed);
    lua_register(L, "mouse_released", mouse_released);
    lua_register(L, "mouse_pos", mouse_pos);
    lua_register(L, "scroll", scroll);

    // sound
    lua_register(L, "sfx", sfx);
    lua_register(L, "sfx_play", sfx);
    lua_register(L, "sfx_loop", sfx_loop);
    lua_register(L, "sfx_stop", sfx_stop);

    // screen
    lua_register(L, "settransparent", settransparent);
    lua_register(L, "clear", clear);
    lua_register(L, "pix", pix);
    lua_register(L, "write", write);
    lua_register(L, "spr", spr);

    // map
    lua_register(L, "get_tile", get_tile);
    lua_register(L, "set_tile", set_tile);
    lua_register(L, "maprender", maprender);

    return 0;
}

int luag_lib_destroy(void) {
    return 0;
}
