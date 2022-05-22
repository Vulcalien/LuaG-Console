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
#include "lua-engine.h"
#include "map.h"
#include "input.h"
#include "sound.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>
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

#ifdef _WIN32
    #include <fileapi.h>

    char *realpath(char *path, char *resolved_path) {
        bool is_allocated = (resolved_path == NULL);
        if(is_allocated)
            resolved_path = malloc(PATH_MAX * sizeof(char));

        if(!GetFullPathNameA(
            path, PATH_MAX,
            resolved_path, NULL
        )) {
            if(is_allocated)
                free(resolved_path);
            return NULL;
        }
        return resolved_path;
    }
#endif

// generic
F(loadscript) {
    const char *script_filename = luaL_checkstring(L, 1);

    char *file_rel_path        = malloc(PATH_MAX * sizeof(char));
    char *file_abs_path        = NULL;
    char *game_folder_abs_path = NULL;

    snprintf(
        file_rel_path, PATH_MAX,
        "%s/scripts/%s", game_folder, script_filename
    );

    file_abs_path = realpath(file_rel_path, NULL);
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

    game_folder_abs_path = realpath(game_folder, NULL);
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
    if(file_abs_path)
        free(file_abs_path);
    if(game_folder_abs_path)
        free(game_folder_abs_path);
    return 0;
}

F(luag_exit) {
    lua_Integer code = luaL_optinteger(L, 1, 0);
    const char *msg  = luaL_optstring(L, 2, "");

    engine_ask_exit(code, msg);
    return 0;
}

F(luag_log) {
    // TODO
    return 0;
}

// keys
static struct input_Key *get_key(lua_State *L, bool is_string,
                                 const char *key_name, lua_Integer id) {
    if(is_string) {
        u32 len = strlen(key_name);
        char *string = malloc((len + 1) * sizeof(char));

        // copy while making lowercase
        for(u32 i = 0; i < len; i++)
            string[i] = tolower(key_name[i]);
        string[len] = '\0';

        #define TEST(key_name) !strcmp(string, key_name)

        if(TEST("up"))
            id = KEY_UP;
        else if(TEST("left"))
            id = KEY_LEFT;
        else if(TEST("down"))
            id = KEY_DOWN;
        else if(TEST("right"))
            id = KEY_RIGHT;
        else if(TEST("a"))
            id = KEY_A;
        else if(TEST("b"))
            id = KEY_B;
        else if(TEST("start"))
            id = KEY_START;
        else if(TEST("select"))
            id = KEY_SELECT;
        else
            id = -1;

        #undef TEST

        if(id < 0)
            throw_lua_error(L, "bad argument: key '%s' does not exist", string);

        free(string);

        if(id < 0)
            return NULL;
    } else {
        if(id < 0 || id >= KEY_COUNT) {
            throw_lua_error(L, "bad argument: key '%d' does not exist", id);
            return NULL;
        }
    }

    return &input_keys[id];
}

F(key) {
    struct input_Key *key;
    if(lua_isinteger(L, 1))
        key = get_key(L, false, NULL, luaL_checkinteger(L, 1));
    else
        key = get_key(L, true, luaL_checkstring(L, 1), 0);

    if(!key)
        return 0;

    lua_pushboolean(L, key->is_down);
    return 1;
}

F(key_pressed) {
    struct input_Key *key;
    if(lua_isinteger(L, 1))
        key = get_key(L, false, NULL, luaL_checkinteger(L, 1));
    else
        key = get_key(L, true, luaL_checkstring(L, 1), 0);

    if(!key)
        return 0;

    lua_pushboolean(L, key->press_count);
    return 1;
}

F(key_released) {
    struct input_Key *key;
    if(lua_isinteger(L, 1))
        key = get_key(L, false, NULL, luaL_checkinteger(L, 1));
    else
        key = get_key(L, true, luaL_checkstring(L, 1), 0);

    if(!key)
        return 0;

    lua_pushboolean(L, key->release_count);
    return 1;
}

// mouse
static struct input_Key *get_btn(lua_State *L, lua_Integer id) {
    if(id < 0 || id >= BTN_COUNT) {
        throw_lua_error(L, "bad argument: button '%d' does not exist", id);
        return NULL;
    }

    return &input_btns[id];
}

F(mouse) {
    struct input_Key *btn = get_btn(L, luaL_checkinteger(L, 1));
    if(!btn)
        return 0;

    lua_pushboolean(L, btn->is_down);
    return 1;
}

F(mouse_pressed) {
    struct input_Key *btn = get_btn(L, luaL_checkinteger(L, 1));
    if(!btn)
        return 0;

    lua_pushboolean(L, btn->press_count);
    return 1;
}

F(mouse_released) {
    struct input_Key *btn = get_btn(L, luaL_checkinteger(L, 1));
    if(!btn)
        return 0;

    lua_pushboolean(L, btn->release_count);
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

    lua_Integer w = 1;
    lua_Integer h = 1;
    lua_Integer alpha = 0xff;

    if(!lua_isnoneornil(L, 4)) {
        luaL_checktype(L, 4, LUA_TTABLE);

        lua_getfield(L, 4, "w");
        w = luaL_optinteger(L, -1, w);

        lua_getfield(L, 4, "h");
        h = luaL_optinteger(L, -1, h);

        lua_getfield(L, 4, "alpha");
        alpha = luaL_optinteger(L, -1, alpha);
    }

    display_fill(x, y, w, h, color, alpha);
    return 0;
}

F(write) {
    const char *text  = luaL_checkstring(L, 1);
    lua_Integer color = luaL_checkinteger(L, 2);
    lua_Integer x     = luaL_checkinteger(L, 3);
    lua_Integer y     = luaL_checkinteger(L, 4);

    lua_Integer alpha = 0xff;

    if(!lua_isnoneornil(L, 5)) {
        luaL_checktype(L, 5, LUA_TTABLE);

        lua_getfield(L, 5, "alpha");
        alpha = luaL_optinteger(L, -1, alpha);
    }

    display_write(text, color, x, y, 1, alpha);
    return 0;
}

F(spr) {
    lua_Integer id = luaL_checkinteger(L, 1);
    lua_Integer x  = luaL_checkinteger(L, 2);
    lua_Integer y  = luaL_checkinteger(L, 3);

    lua_Integer scale = 1;

    lua_Integer sw = 1;
    lua_Integer sh = 1;

    lua_Integer rot = 0;

    bool h_flip = false;
    bool v_flip = false;

    lua_Integer col_mod = 0xffffff;
    lua_Integer alpha = 0xff;

    if(!lua_isnoneornil(L, 4)) {
        luaL_checktype(L, 4, LUA_TTABLE);

        lua_getfield(L, 4, "scale");
        scale = luaL_optinteger(L, -1, scale);

        lua_getfield(L, 4, "sw");
        sw = luaL_optinteger(L, -1, sw);

        lua_getfield(L, 4, "sh");
        sh = luaL_optinteger(L, -1, sh);

        lua_getfield(L, 4, "rot");
        rot = luaL_optinteger(L, -1, rot);

        lua_getfield(L, 4, "h_flip");
        h_flip = luaL_opt(L, lua_toboolean, -1, h_flip);

        lua_getfield(L, 4, "v_flip");
        v_flip = luaL_opt(L, lua_toboolean, -1, v_flip);

        lua_getfield(L, 4, "col_mod");
        col_mod = luaL_optinteger(L, -1, col_mod);

        lua_getfield(L, 4, "alpha");
        alpha = luaL_optinteger(L, -1, alpha);
    }

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
            alpha, col_mod
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
        return 0;
    } else {
        u8 old_tile = map_get_tile(x, y);
        map_set_tile(x, y, id);
        lua_pushboolean(L, old_tile != id);
        return 1;
    }
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

// time
F(luag_time) {
    lua_pushinteger(L, time(NULL));
    return 1;
}

F(luag_date) {
    // not equivalent to luaL_optinteger: here, time(NULL)
    // is not called if the argument is set
    time_t time_to_use = luaL_opt(L, luaL_checkinteger, 1, time(NULL));
    struct tm *t = localtime(&time_to_use);

    lua_createtable(L, 0, 7);

    lua_pushinteger(L, t->tm_sec);
    lua_setfield(L, -2, "sec");

    lua_pushinteger(L, t->tm_min);
    lua_setfield(L, -2, "min");

    lua_pushinteger(L, t->tm_hour);
    lua_setfield(L, -2, "hour");

    lua_pushinteger(L, t->tm_mday);
    lua_setfield(L, -2, "day");

    lua_pushinteger(L, t->tm_mon + 1);
    lua_setfield(L, -2, "month");

    lua_pushinteger(L, t->tm_year + 1900);
    lua_setfield(L, -2, "year");

    lua_pushinteger(L, t->tm_wday + 1);
    lua_setfield(L, -2, "wday");

    lua_pushinteger(L, t->tm_yday + 1);
    lua_setfield(L, -2, "yday");

    return 1;
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
    lua_register(L, "exit", luag_exit);
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

    // time
    lua_register(L, "time", luag_time);
    lua_register(L, "date", luag_date);

    return 0;
}

int luag_lib_destroy(void) {
    return 0;
}
