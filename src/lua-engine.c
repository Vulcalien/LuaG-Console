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

static void *core_lib_handle = NULL;
static void *editor_lib_handle = NULL;

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

static int run_library_function(lua_State *L, void *handle, char *name) {
    int (*func)(lua_State *L);
    *(void **) (&func) = dlsym(handle, name);

    if(!func) {
        fprintf(
            stderr,
            "Engine: the LuaG Library object does not contain the function "
            "'%s'\n", name
        );
        return -1;
    }

    int status = func(L);
    if(status) {
        fprintf(stderr, "Engine: '%s' returned %d\n", name, status);
        return -2;
    }
    return 0;
}

static void *load_luag_library(lua_State *L, bool is_editor_lib) {
    void *handle;

    if(!is_editor_lib) {
        char *filename = malloc(PATH_MAX * sizeof(char));

        // check at most 100 times
        // i'm pretty sure there is a better way
        for(u32 i = 0; i < 100; i++) {
            snprintf(
                filename, PATH_MAX,
                RESOURCES_DIR "/luag-lib/luag-lib-%d.%d.so",
                cartridge_info.major_v, (cartridge_info.minor_v + i)
            );

            handle = dlopen(filename, RTLD_LAZY);
            if(handle)
                break;
        }
        free(filename);

        if(!handle) {
            fprintf(
                stderr,
                "Engine: could not find a version of LuaG Library compatible "
                "with %d.%d\n",
                cartridge_info.major_v, cartridge_info.minor_v
            );
            return NULL;
        }
    } else {
        handle = dlopen(
            RESOURCES_DIR "/luag-lib/luag-lib-editor.so",
            RTLD_LAZY
        );

        if(!handle) {
            fputs(
                "Engine: could not find the LuaG Editor Library\n",
                stderr
            );
            return NULL;
        }
    }

    if(run_library_function(L, handle, "luag_lib_load")) {
        dlclose(handle);
        return NULL;
    }

    return handle;
}

static void destroy_luag_library(void *handle) {
    run_library_function(L, handle, "luag_lib_destroy");
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

void engine_load(bool is_editor) {
    if(engine_running) {
        fputs("Engine: engine is running when calling 'engine_load'\n", stderr);
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

    core_lib_handle = load_luag_library(L, false);
    if(!core_lib_handle) {
        fputs("Engine: could not load LuaG Library\n", stderr);
        engine_stop();
        return;
    }

    if(is_editor) {
        editor_lib_handle = load_luag_library(L, true);
        if(!editor_lib_handle) {
            fputs("Engine: could not load LuaG Editor Library\n", stderr);
            engine_stop();
            return;
        }
    }

    if(load_main_file())
        return;

    // run "init" function
    lua_getglobal(L, "init");
    if(!lua_isfunction(L, -1)) {
        fputs("Engine: 'main.lua' must contain a function 'init()'\n", stderr);
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
            "Engine: engine is not running when calling 'engine_stop'\n",
            stderr
        );
        return;
    }
    engine_running = false;

    if(L) {
        lua_close(L);
        L = NULL;
    }

    if(core_lib_handle)
        destroy_luag_library(core_lib_handle);

    if(editor_lib_handle)
        destroy_luag_library(editor_lib_handle);

    input_set_text_mode(true);
}

void engine_tick(void) {
    lua_getglobal(L, "tick");
    if(!lua_isfunction(L, -1)) {
        fputs("Engine: a function 'tick()' must be defined\n", stderr);
        engine_stop();
        return;
    }

    int status = lua_pcall(L, 0, 0, 0);
    if(check_error(L, status))
        return;
}
