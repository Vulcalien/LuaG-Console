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
#include "lua-engine.h"

#include "terminal.h"
#include "input.h"
#include "cartridge.h"

#include <stdio.h>
#include <limits.h>
#include <dlfcn.h>

#include <lua5.4/lua.h>
#include <lua5.4/lualib.h>
#include <lua5.4/lauxlib.h>

#define BUFFER_SIZE (4 * 1024)

bool engine_running = false;

static lua_State *L = NULL;
static void *shared_lib_handle = NULL;

// returns nonzero if the engine was stopped
static int check_error(lua_State *L, int status) {
    if(status != LUA_OK) {
        const char *error_msg = lua_tostring(L, -1);

        fprintf(stderr, "%s\n", error_msg);

        lua_pop(L, 1);

        engine_stop();
        return -1;
    }
    return 0;
}

static int load_luag_library(lua_State *L) {
    char *filename = malloc(PATH_MAX * sizeof(char));

    // check at most 100 times
    // i'm pretty sure there is a better way
    for(u32 i = 0; i < 100; i++) {
        snprintf(
            filename, PATH_MAX,
            RESOURCES_DIR "/luag-lib/luag-lib-%d.%d.so",
            cartridge_info.major_v, (cartridge_info.minor_v + i)
        );

        shared_lib_handle = dlopen(filename, RTLD_LAZY);
        if(shared_lib_handle)
            break;
    }
    free(filename);

    if(!shared_lib_handle) {
        fprintf(
            stderr,
            "Error: could not find a version of LuaG library compatible "
            "with %d.%d\n",
            cartridge_info.major_v, cartridge_info.minor_v
        );
        return -1;
    }

    // call luag_lib_load
    int (*lib_load_fn)(lua_State *L);
    *(void **) (&lib_load_fn) = dlsym(shared_lib_handle, "luag_lib_load");

    if(!lib_load_fn) {
        fputs(
            "Error: the LuaG Library object does not contain the function "
            "'luag_lib_load'\n",
            stderr
        );
        dlclose(shared_lib_handle);
        return -2;
    }
    int status = lib_load_fn(L);
    if(status) {
        fprintf(stderr, "Error: 'luag_lib_load' returned %d\n", status);
        return -3;
    }

    return 0;
}

static void destroy_luag_library(void *handle) {
    int (*lib_destroy_fn)(void);
    *(void **) (&lib_destroy_fn) = dlsym(handle, "luag_lib_destroy");

    if(!lib_destroy_fn) {
        fputs(
            "Error: the LuaG Library object does not contain the function "
            "'luag_lib_destroy'\n",
            stderr
        );
    }

    int status = lib_destroy_fn();
    if(status)
        fprintf(stderr, "Error: 'luag_lib_destroy' returned %d\n", status);

    dlclose(handle);
}

static int load_main_file(void) {
    char *filename = malloc(PATH_MAX * sizeof(char));
    snprintf(
        filename, PATH_MAX,
        "%s/scripts/main.lua", game_folder
    );

    int status = luaL_dofile(L, filename);

    free(filename);

    return check_error(L, status);
}

void engine_load(void) {
    if(engine_running) {
        fputs("Error: engine is running when calling 'engine_load'\n", stderr);
        return;
    }
    engine_running = true;

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

    if(cartridge_load_files()) {
        engine_stop();
        return;
    }

    if(load_luag_library(L)) {
        fputs("Error: could not load LuaG Library\n", stderr);
        engine_stop();
        return;
    }

    if(load_main_file())
        return;

    // run "init" function
    lua_getglobal(L, "init");
    if(!lua_isfunction(L, -1)) {
        fputs("Error: 'main.lua' must contain a function 'init()'\n", stderr);

        engine_stop();
        return;
    }

    int status = lua_pcall(L, 0, 0, 0);
    if(check_error(L, status))
        return;

    input_set_text_mode(false);
}

void engine_stop(void) {
    if(!engine_running) {
        fputs(
            "Error: engine is not running when calling 'engine_stop'\n",
            stderr
        );
        return;
    }
    engine_running = false;

    if(L) {
        lua_close(L);
        L = NULL;
    }

    if(shared_lib_handle)
        destroy_luag_library(shared_lib_handle);

    input_set_text_mode(true);
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
