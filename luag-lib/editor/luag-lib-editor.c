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

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <lua5.4/lua.h>
#include <lua5.4/lauxlib.h>

#include "display.h"
#include "map.h"

#define F(name) static int name(lua_State *L)

static SDL_Surface *atlas_surface = NULL;
static SDL_Texture *atlas_texture = NULL;

static void throw_lua_error(lua_State *L, char *msg_format, ...) {
    va_list args;
    va_start(args, msg_format);

    luaL_where(L, 1);
    lua_pushvfstring(L, msg_format, args);
    lua_concat(L, 2);
    lua_error(L);

    va_end(args);
}

static int load_atlas(char *filename) {
    return display_load_atlas(filename, &atlas_surface, &atlas_texture);
}

static void load_map(char *filename) {
    if(map_load(filename)) {
        map = (struct Map) {
            .width = 10,
            .height = 10,
            .tiles = calloc(10 * 10, sizeof(u8))
        };
    }
}

F(editor_load_files) {
    int err = 0;

    if(load_atlas(USERDATA_FOLDER "/atlas.png")) {
        err = -1;
        goto exit;
    }

    load_map(USERDATA_FOLDER "/map");

    exit:
    lua_pushinteger(L, err);
    return 1;
}

F(editor_spr) {
    lua_Integer id = luaL_checkinteger(L, 1);
    lua_Integer x  = luaL_checkinteger(L, 2);
    lua_Integer y  = luaL_checkinteger(L, 3);

    lua_Integer scale = lua_isnoneornil(L, 4)
                        ? 1 : luaL_checkinteger(L, 4);

    lua_Integer sw = lua_isnoneornil(L, 5)
                     ? 1 : luaL_checkinteger(L, 5);
    lua_Integer sh = lua_isnoneornil(L, 6)
                     ? 1 : luaL_checkinteger(L, 6);

    char *err_msg = NULL;
    if(id < 0 || id >= 256)
        err_msg = "bad argument: id";
    else if(sw <= 0 || (id % 16) + sw > 16)
        err_msg = "bad argument: sw";
    else if(sh <= 0 || (id / 16) + sh > 16)
        err_msg = "bad argument: sh";

    if(err_msg) {
        throw_lua_error(L, err_msg);
    } else {
        display_draw_from_atlas(
            atlas_texture,
            id, x, y,
            scale, sw, sh,
            0, false, false,
            0xffffff
        );
    }
    return 0;
}

F(editor_maprender) {
    lua_Integer scale = lua_isnoneornil(L, 1)
                        ? 1 : luaL_checkinteger(L, 1);

    lua_Integer xoff = lua_isnoneornil(L, 2)
                       ? 0 : luaL_checkinteger(L, 2);
    lua_Integer yoff = lua_isnoneornil(L, 3)
                       ? 0 : luaL_checkinteger(L, 3);

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
                    atlas_texture,
                    id, xt * tile_size - xoff, yt * tile_size - yoff,
                    scale, 1, 1,
                    0, false, false,
                    0xffffff
                );
            }
        }
    }
    return 0;
}

F(editor_draw_atlas) {
    // draw coordinates
    lua_Integer x0 = luaL_checkinteger(L, 1);
    lua_Integer y0 = luaL_checkinteger(L, 2);

    lua_Integer row0 = luaL_checkinteger(L, 3);
    lua_Integer rows = luaL_checkinteger(L, 4);

    char *err_msg = NULL;
    if(row0 < 0 || row0 >= 16)
        err_msg = "bad argument: row0";
    else if(rows <= 0 || row0 + rows > 16)
        err_msg = "bad argument: rows";

    if(err_msg) {
        throw_lua_error(L, err_msg);
    } else {
        for(u32 ys = row0; ys < row0 + rows; ys++) {
            for(u32 xs = 0; xs < 16; xs++) {
                u32 id = xs + ys * 16;
                display_draw_from_atlas(
                    atlas_texture,
                    id, x0 + xs * SPRITE_SIZE, y0 + ys * SPRITE_SIZE,
                    1, 1, 1,
                    0, false, false,
                    0xffffff
                );
            }
        }
    }
    return 0;
}

int luag_lib_load(lua_State *L) {
    lua_register(L, "editor_load_files", editor_load_files);

    lua_register(L, "editor_spr", editor_spr);
    lua_register(L, "editor_maprender", editor_maprender);

    lua_register(L, "editor_draw_atlas", editor_draw_atlas);

    return 0;
}

int luag_lib_destroy(void) {
    if(atlas_surface)
        SDL_FreeSurface(atlas_surface);

    return 0;
}
