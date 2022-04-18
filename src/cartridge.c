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
#include "cartridge.h"

#include "terminal.h"
#include "display.h"
#include "map.h"
#include "sound.h"
#include "archive-util.h"

#include <stdio.h>
#include <limits.h>

#include <sys/stat.h>
#include <sys/types.h>

#ifdef __unix__
    #define TEMP_DIR "/tmp/luag-console"
#elif _WIN32
    #include <fileapi.h>

    static char *temp_dir;
#endif

static int load_cartridge_info(void);
static int load_atlas(void);
static int load_map(void);
static int load_sounds(void);

struct cartridge_Info cartridge_info;

static char *cartridge_folder;

int cartridge_init(void) {
    #ifdef __unix__
        mkdir(TEMP_DIR, 0700);
    #elif _WIN32
        char temp_dir_win[PATH_MAX];
        GetTempPathA(PATH_MAX, temp_dir_win);

        temp_dir = malloc(PATH_MAX * sizeof(char));
        snprintf(temp_dir, PATH_MAX, "%sluag-console", temp_dir_win);

        mkdir(temp_dir);
    #endif

    return 0;
}

void cartridge_destroy(void) {
    if(cartridge_folder)
        free(cartridge_folder);

    #ifdef _WIN32
        free(temp_dir);
    #endif
}

char *cartridge_extract(const char *filename) {
    // prepare temporary folder
    if(!cartridge_folder)
        cartridge_folder = malloc(PATH_MAX * sizeof(char));

    #ifdef __unix__
        snprintf(cartridge_folder, PATH_MAX, TEMP_DIR "/XXXXXX");
        cartridge_folder = mkdtemp(cartridge_folder);
    #elif _WIN32
        #define CHAR_POOL "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
        #define CHAR_POOL_LEN (sizeof(CHAR_POOL) / sizeof(char) - 1)

        #define NAME_BUFFER_SIZE 16
        char random_name[NAME_BUFFER_SIZE + 1];

        for(u32 i = 0; i < NAME_BUFFER_SIZE; i++)
            random_name[i] = CHAR_POOL[rand() % CHAR_POOL_LEN];
        random_name[NAME_BUFFER_SIZE] = '\0';

        snprintf(
            cartridge_folder, PATH_MAX,
            "%s/%s", temp_dir, random_name
        );

        #undef CHAR_POOL
        #undef CHAR_POOL_LEN
        #undef NAME_BUFFER_SIZE
    #endif

    if(archiveutil_extract(filename, cartridge_folder))
        return NULL;

    return cartridge_folder;
}

int cartridge_load_files(void) {
    if(load_cartridge_info())
        return -1;
    if(load_atlas())
        return -2;
    if(load_map())
        return -3;
    if(load_sounds())
        return -4;
    return 0;
}

static int load_cartridge_info(void) {
    char filename[PATH_MAX];
    snprintf(filename, PATH_MAX, "%s/cartridge-info", game_folder);

    FILE *file = fopen(filename, "r");

    if(!file) {
        // defaults
        cartridge_info.major_v = CARTRIDGE_DEFAULT_MAJOR_V;
        cartridge_info.minor_v = CARTRIDGE_DEFAULT_MINOR_V;
        return 0;
    }

    int result = fscanf(
        file,
        " library-version = %u.%u",
        &cartridge_info.major_v,
        &cartridge_info.minor_v
    );

    int err = 0;
    if(result != 2) {
        terminal_write(
            "Error:\n"
            "'cartridge-info'\n"
            "is invalid",
            true
        );
        err = -1;
    }

    fclose(file);
    return err;
}

static int load_atlas(void) {
    char filename[PATH_MAX];
    snprintf(filename, PATH_MAX, "%s/atlas.png", game_folder);

    return display_load_atlas(filename, NULL, NULL);
}

static int load_map(void) {
    char filename[PATH_MAX];
    snprintf(filename, PATH_MAX, "%s/map", game_folder);

    return map_load(filename);
}

static int load_sounds(void) {
    char folder[PATH_MAX];
    snprintf(folder, PATH_MAX, "%s/sfx", game_folder);

    return sound_load(folder);
}
