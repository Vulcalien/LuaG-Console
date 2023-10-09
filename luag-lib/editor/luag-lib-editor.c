/* Copyright 2022-2023 Vulcalien
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

#include <string.h>
#include <limits.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <lua5.4/lua.h>
#include <lua5.4/lauxlib.h>

#include "display.h"
#include "input.h"
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

static inline u32 *atlas_get_row(u32 y) {
    u8 *pixels = atlas_surface->pixels;
    u32 pitch = atlas_surface->pitch;
    return (u32 *)(pixels + pitch * y);
}

static int load_atlas(lua_State *L, char *filename) {
    SDL_Surface *tmp_surface = NULL;
    if(display_load_atlas(filename, &tmp_surface, &atlas_texture)) {
        return -1;
    }

    // convert the atlas surface to RGB to allow easy pixel edits
    if(tmp_surface) {
        atlas_surface = SDL_ConvertSurfaceFormat(
            tmp_surface, SDL_PIXELFORMAT_RGB888, 0
        );
        SDL_FreeSurface(tmp_surface);

        if(!atlas_surface) {
            fprintf(
                stderr,
                "SDL: could not convert editor's atlas surface to RGB\n"
                " - SDL_ConvertSurfaceFormat: %s\n",
                SDL_GetError()
            );
            return -2;
        }
    }

    return 0;
}

static void load_map(lua_State *L, char *filename) {
    if(map_load(filename)) {
        map = (struct Map) {
            .width = 10,
            .height = 10,
            .tiles = calloc(10 * 10, sizeof(u8))
        };
    }

    // update value of map_w and map_h
    lua_pushinteger(L, map.width);
    lua_setglobal(L, "map_w");

    lua_pushinteger(L, map.height);
    lua_setglobal(L, "map_h");
}

F(editor_load_files) {
    int err = 0;

    if(load_atlas(L, USERDATA_FOLDER "/atlas.png")) {
        err = -1;
        goto exit;
    }

    load_map(L, USERDATA_FOLDER "/map");

    exit:
    lua_pushinteger(L, err);
    return 1;
}

static int fwrite_u32_big_endian(u32 num, FILE *file) {
    u8 b[4] = { num >> 24, num >> 16, num >> 8, num };

    if(fwrite(b, sizeof(u8), 4, file) < 4)
        return -1;

    return 0;
}

F(editor_save_map) {
    int err = 0;

    #define filename (USERDATA_FOLDER "/map")
    FILE *file = fopen(filename, "wb");

    if(!file) {
        fprintf(stderr, "Editor: could not create map file '%s'\n", filename);
        err = -1;
        goto exit;
    }
    #undef filename

    if(fwrite_u32_big_endian(map.width, file) ||
       fwrite_u32_big_endian(map.height, file)) {
        fputs("Editor: could not write map header\n", stderr);
        goto exit;
    }

    u32 map_size = map.width * map.height;
    if(fwrite(map.tiles, sizeof(u8), map_size, file) < map_size) {
        fputs("Editor: could not write map content\n", stderr);
        goto exit;
    }

    exit:
    if(file)
        fclose(file);

    lua_pushinteger(L, err);
    return 1;
}

F(editor_save_atlas) {
    #define filename (USERDATA_FOLDER "/atlas.png")
    int err = IMG_SavePNG(atlas_surface, filename);

    if(err) {
        fprintf(
            stderr,
            "Editor: could not create atlas file '%s'\n",
            filename
        );
    }
    #undef filename

    lua_pushinteger(L, err);
    return 1;
}

F(editor_update_map_size) {
    lua_Integer w        = luaL_checkinteger(L, 1);
    lua_Integer h        = luaL_checkinteger(L, 2);
    lua_Integer selected = luaL_checkinteger(L, 3);

    if(w >= 0) {
        if(w > 8192)
            w = 8192;
    } else {
        w = map.width;
    }

    if(h >= 0) {
        if(h > 8192)
            h = 8192;
    } else {
        h = map.height;
    }

    u8 *new_tiles = malloc((w * h) * sizeof(u8));

    memset(new_tiles, selected, (w * h) * sizeof(u8));

    u32 row_len = map.width < w ? map.width : w;
    u32 col_len = map.height < h ? map.height : h;
    for(u32 i = 0; i < col_len; i++) {
        memcpy(
            new_tiles + w * i,
            map.tiles + map.width * i,
            row_len * sizeof(u8)
        );
    }

    free(map.tiles);
    map = (struct Map) {
        .width = w,
        .height = h,
        .tiles = new_tiles
    };

    // update map_w and map_h
    lua_pushinteger(L, map.width);
    lua_setglobal(L, "map_w");

    lua_pushinteger(L, map.height);
    lua_setglobal(L, "map_h");

    return 0;
}

F(editor_atlas_set_pixel) {
    lua_Integer x     = luaL_checkinteger(L, 1);
    lua_Integer y     = luaL_checkinteger(L, 2);
    lua_Integer color = luaL_checkinteger(L, 3);

    if(x < 0 || x >= atlas_surface->w) {
        throw_lua_error(L, "bad argument: x");
        return 0;
    }
    if(y < 0 || y >= atlas_surface->h) {
        throw_lua_error(L, "bad argument: y");
        return 0;
    }

    u32 *row = atlas_get_row(y);
    row[x] = color;

    // update atlas_texture
    display_update_atlas(atlas_surface, &atlas_texture);

    return 0;
}

F(editor_atlas_get_pixel) {
    lua_Integer x = luaL_checkinteger(L, 1);
    lua_Integer y = luaL_checkinteger(L, 2);

    if(x < 0 || x >= atlas_surface->w) {
        throw_lua_error(L, "bad argument: x");
        return 0;
    }
    if(y < 0 || y >= atlas_surface->h) {
        throw_lua_error(L, "bad argument: y");
        return 0;
    }

    u32 *row = atlas_get_row(y);
    lua_pushinteger(L, row[x]);

    return 1;
}

// If the pixel (x, y) is the same as 'target_color' and (x, y) is
// within the bounds (x0, y0, x1, y1), set the pixel (x, y) to 'color'
// and recoursively try to do the same for adiacent pixels.
static void fill(i32 x, i32 y, u32 color, u32 target_color,
                 i32 x0, i32 y0, i32 x1, i32 y1) {
    // check if (x, y) is within bounds
    if(x < x0 || y < y0 || x > x1 || y > y1)
        return;

    u32 *row = atlas_get_row(y);

    // check if (x, y) has the color 'target_color'
    if(row[x] != target_color)
        return;

    row[x] = color;

    // recoursively try to do the same for adiacent pixels
    fill(x - 1, y,     color, target_color, x0, y0, x1, y1);
    fill(x,     y - 1, color, target_color, x0, y0, x1, y1);
    fill(x + 1, y,     color, target_color, x0, y0, x1, y1);
    fill(x,     y + 1, color, target_color, x0, y0, x1, y1);
}

F(editor_atlas_fill) {
    lua_Integer x     = luaL_checkinteger(L, 1);
    lua_Integer y     = luaL_checkinteger(L, 2);
    lua_Integer color = luaL_checkinteger(L, 3);
    lua_Integer x0    = luaL_checkinteger(L, 4);
    lua_Integer y0    = luaL_checkinteger(L, 5);
    lua_Integer x1    = luaL_checkinteger(L, 6);
    lua_Integer y1    = luaL_checkinteger(L, 7);

    if(x < 0 || x >= atlas_surface->w) {
        throw_lua_error(L, "bad argument: x");
        return 0;
    }
    if(y < 0 || y >= atlas_surface->h) {
        throw_lua_error(L, "bad argument: y");
        return 0;
    }

    if(x0 < 0 || x0 >= atlas_surface->w) {
        throw_lua_error(L, "bad argument: x0");
        return 0;
    }
    if(y0 < 0 || y0 >= atlas_surface->h) {
        throw_lua_error(L, "bad argument: y0");
        return 0;
    }
    if(x1 < 0 || x1 >= atlas_surface->w) {
        throw_lua_error(L, "bad argument: x1");
        return 0;
    }
    if(y1 < 0 || y1 >= atlas_surface->h) {
        throw_lua_error(L, "bad argument: y1");
        return 0;
    }

    // read the target pixel
    u32 *row = atlas_get_row(y);
    u32 target_color = row[x];

    // if 'target_color' is the same as 'color', do nothing
    if(target_color == color)
        return 0;

    // fill and update atlas_texture
    fill(x, y, color, target_color, x0, y0, x1, y1);
    display_update_atlas(atlas_surface, &atlas_texture);

    return 0;
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
            0xff, 0xffffff
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
                    0xff, 0xffffff
                );
            }
        }
    }
    return 0;
}

F(editor_set_text_mode) {
    bool flag = lua_toboolean(L, -1);

    input_set_text_mode(flag);
    return 0;
}

F(editor_get_text) {
    lua_pushstring(L, input_get_text());
    return 1;
}

int luag_lib_load(lua_State *L) {
    lua_register(L, "editor_load_files", editor_load_files);

    lua_register(L, "editor_save_map", editor_save_map);
    lua_register(L, "editor_save_atlas", editor_save_atlas);

    lua_register(L, "editor_update_map_size", editor_update_map_size);

    lua_register(L, "editor_atlas_set_pixel", editor_atlas_set_pixel);
    lua_register(L, "editor_atlas_get_pixel", editor_atlas_get_pixel);
    lua_register(L, "editor_atlas_fill", editor_atlas_fill);

    lua_register(L, "editor_spr", editor_spr);
    lua_register(L, "editor_maprender", editor_maprender);

    lua_register(L, "editor_set_text_mode", editor_set_text_mode);
    lua_register(L, "editor_get_text", editor_get_text);

    return 0;
}

int luag_lib_destroy(void) {
    if(atlas_surface)
        SDL_FreeSurface(atlas_surface);
    if(atlas_texture)
        SDL_DestroyTexture(atlas_texture);

    return 0;
}
