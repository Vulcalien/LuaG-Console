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
    atlas_surface = IMG_Load(filename);
    if(!atlas_surface) {
        fprintf(stderr, "Editor: cannot load atlas file %s\n", filename);
        return -1;
    }

    if(atlas_surface->w != 128 || atlas_surface->h != 128) {
        fprintf(
            stderr,
            "Editor: atlas is of wrong size: "
            "(%d, %d) instead of (128, 128)\n",
            atlas_surface->w, atlas_surface->h
        );

        SDL_FreeSurface(atlas_surface);
        atlas_surface = NULL;

        return -2;
    }
    return 0;
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

int luag_lib_load(lua_State *L) {
    lua_register(L, "editor_load_files", editor_load_files);

    return 0;
}

int luag_lib_destroy(void) {
    if(atlas_surface)
        SDL_FreeSurface(atlas_surface);

    return 0;
}
