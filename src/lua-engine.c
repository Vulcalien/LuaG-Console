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
#include "sound.h"
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
            "Engine: could not find function '%s'\n"
            " - dlsym: %s\n",
            name, dlerror()
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
        char filename[PATH_MAX];

        // check at most 100 times
        // i'm pretty sure there is a better way
        for(u32 i = 0; i < 100; i++) {
            snprintf(
                filename, PATH_MAX,
                "%s/luag-lib/luag-lib-%d.%d.so",
                res_folder,
                cartridge_info.major_v, (cartridge_info.minor_v + i)
            );

            handle = dlopen(filename, RTLD_LAZY);
            if(handle) {
                printf("Loading LuaG Library: %s\n", filename);
                break;
            }
        }

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
        char filename[PATH_MAX];
        snprintf(
            filename, PATH_MAX,
            "%s/luag-lib/luag-lib-editor.so", res_folder
        );

        handle = dlopen(filename, RTLD_LAZY);
        if(handle)
            printf("Loading LuaG Editor Library: %s\n", filename);

        if(!handle) {
            fprintf(
                stderr,
                "Engine: could not find the LuaG Editor Library\n"
                " - dlopen: %s\n", dlerror()
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
    char filename[PATH_MAX];
    snprintf(
        filename, PATH_MAX,
        "%s/scripts/main.lua", game_folder
    );

    int status = luaL_dofile(L, filename);

    return check_error(L, status);
}

#define lua_load(func, name) luaL_requiref(L, name, func, 1); lua_pop(L, 1)

void engine_load(bool is_editor) {
    if(engine_running) {
        fputs("Engine: engine is running when calling 'engine_load'\n", stderr);
        return;
    }
    engine_running = true;

    L = luaL_newstate();

    // load libraries

    lua_load(luaopen_base, LUA_GNAME);
    // coroutine
    // package
    lua_load(luaopen_string, LUA_STRLIBNAME);
    // UTF-8
    lua_load(luaopen_table, LUA_TABLIBNAME);
    lua_load(luaopen_math, LUA_MATHLIBNAME);
    // io
    // os
    // debug

    // TODO luaopen_base loads functions like dofile.
    // delete them

    if(cartridge_load_files()) {
        engine_stop();
        return;
    }
    input_reset_keys();

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

void engine_reload(void) {
    bool is_editor = editor_lib_handle != NULL;

    engine_stop();
    engine_load(is_editor);
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

    if(core_lib_handle) {
        destroy_luag_library(core_lib_handle);
        core_lib_handle = NULL;
    }

    if(editor_lib_handle) {
        destroy_luag_library(editor_lib_handle);
        editor_lib_handle = NULL;
    }

    sound_stop_all();
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
