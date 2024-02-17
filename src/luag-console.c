/* Copyright 2022, 2024 Vulcalien
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

#include "gameloop.h"
#include "lua-engine.h"
#include "terminal.h"
#include "shell-commands.h"
#include "input.h"
#include "input-keys.h"
#include "display.h"
#include "sound.h"
#include "cartridge.h"
#include "map.h"

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static int init(int argc, const char *argv[]);
static void destroy(void);

static int find_res_folder(void);
static int find_config_folder(void);

bool should_quit = false;

bool dev_mode = false;

char *res_folder    = NULL;
char *config_folder = NULL;
char *game_folder   = NULL;

int main(int argc, const char *argv[]) {
    int err = 0;

    err = init(argc, argv);
    if(err)
        goto exit;

    gameloop();

    exit:
    destroy();
    return err;
}

void tick(void) {
    input_tick();

    if(engine_running)
        engine_tick();
    else
        terminal_tick();

    if(should_quit)
        gameloop_stop();
}

void render(void) {
    if(engine_running)
        engine_render();
    else
        terminal_render();

    display_refresh();
}

static int init(int argc, const char *argv[]) {
    if(find_res_folder() || find_config_folder())
        return -1;

    if(display_init())
        return -2;

    input_init();

    if(sound_init())
        return -3;
    if(terminal_init())
        return -4;
    if(commands_init())
        return -5;
    if(cartridge_init())
        return -6;
    if(map_init())
        return -7;
    if(inputkeys_init())
        return -8;

    srand(time(NULL));

    // parse arguments
    for(u32 i = 1; i < argc; i++) {
        const char *arg = argv[i];

        u32 len = strlen(arg);
        if(len == 0)
            continue;

        if(arg[0] == '-') {
            // if the argument starts with a '-' then it's an option

            // TODO add options...
        } else {
            // the argument should be a cartridge path
            terminal_receive_input("run ");
            terminal_receive_input(arg);
            terminal_receive_input("\n");

            terminal_receive_input("exit\n");
        }
    }

    return 0;
}

static void destroy(void) {
    if(engine_running)
        engine_stop();

    sound_destroy();
    input_destroy();
    display_destroy();

    terminal_destroy();
    commands_destroy();
    cartridge_destroy();
    map_destroy();

    if(res_folder)
        free(res_folder);
    if(config_folder)
        free(config_folder);
}

static char *clone_str(char *src) {
    if(src == NULL)
        return NULL;

    char *result = malloc((strlen(src) + 1) * sizeof(char));
    strcpy(result, src);
    return result;
}

static char *find_folder(const char *description, char *list[][2]) {
    char *path = malloc(PATH_MAX * sizeof(char));
    for(u32 i = 0; list[i][0] != NULL; i++) {
        snprintf(
            path, PATH_MAX,
            list[i][0], list[i][1]
        );

        struct stat st;
        if(!stat(path, &st) && S_ISDIR(st.st_mode)) {
            printf("Found %s folder: '%s'\n", description, path);
            return path;
        }
    }
    free(path);

    fprintf(stderr, "LuaG: %s folder not found\n", description);
    return NULL;
}

static int find_res_folder(void) {
    #ifdef __unix__
        char *list[][2] = {
            { NULL }, // $XDG_DATA_HOME or $HOME/.local/share
            { "/usr/share/luag-console" },
            { "/usr/local/share/luag-console" },
            { NULL }
        };

        char *xdg_data_home = clone_str(getenv("XDG_DATA_HOME"));
        if(xdg_data_home && xdg_data_home[0] != '\0') {
            list[0][0] = "%s/luag-console";
            list[0][1] = xdg_data_home;
        } else {
            list[0][0] = "%s/.local/share/luag-console";
            list[0][1] = clone_str(getenv("HOME"));
        }
    #elif _WIN32
        char *list[][2] = {
            { "%s/LuaG Console/res" },
            { "%s/LuaG Console/res" },
            { "%s/LuaG Console/res" },
            { NULL }
        };

        list[0][1] = clone_str(getenv("LOCALAPPDATA"));
        list[1][1] = clone_str(getenv("PROGRAMFILES"));
        list[2][1] = clone_str(getenv("PROGRAMFILES(x86)"));
    #endif

    res_folder = find_folder("resource", list);

    for(u32 i = 0; list[i][0] != NULL; i++) {
        if(list[i][1])
            free(list[i][1]);
    }

    return res_folder == NULL;
}

static int find_config_folder(void) {
    #ifdef __unix__
        char *list[][2] = {
            { NULL }, // $XDG_CONFIG_HOME or $HOME/.config
            { "/etc/luag-console" },
            { NULL }
        };

        char *xdg_config_home = clone_str(getenv("XDG_CONFIG_HOME"));
        if(xdg_config_home && xdg_config_home[0] != '\0') {
            list[0][0] = "%s/luag-console";
            list[0][1] = xdg_config_home;
        } else {
            list[0][0] = "%s/.config/luag-console";
            list[0][1] = clone_str(getenv("HOME"));
        }
    #elif _WIN32
        char *list[][2] = {
            { "%s/LuaG Console/config" },
            { "%s/LuaG Console/config" },
            { "%s/LuaG Console/config" },
            { NULL }
        };

        list[0][1] = clone_str(getenv("LOCALAPPDATA"));
        list[1][1] = clone_str(getenv("PROGRAMFILES"));
        list[2][1] = clone_str(getenv("PROGRAMFILES(x86)"));
    #endif

    config_folder = find_folder("config", list);

    for(u32 i = 0; list[i][0] != NULL; i++) {
        if(list[i][1])
            free(list[i][1]);
    }

    return config_folder == NULL;
}
