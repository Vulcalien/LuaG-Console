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

#define TEMP_DIR "/tmp/luag-console"

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
        mkdir(TEMP_DIR);
    #endif

    return 0;
}

void cartridge_destroy(void) {
    if(cartridge_folder)
        free(cartridge_folder);
}

char *cartridge_extract(const char *filename, char *dest_folder) {
    if(!dest_folder) {
        // prepare temporary folder
        if(!cartridge_folder)
            cartridge_folder = malloc(PATH_MAX * sizeof(char));

        snprintf(cartridge_folder, PATH_MAX, TEMP_DIR "/XXXXXX");
        cartridge_folder = mkdtemp(cartridge_folder);

        dest_folder = cartridge_folder;
    }

    if(archiveutil_extract(filename, dest_folder))
        return NULL;

    return dest_folder;
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
    snprintf(
        filename, PATH_MAX,
        "%s/cartridge-info", game_folder
    );
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
    snprintf(
        filename, PATH_MAX,
        "%s/atlas.png", game_folder
    );
    int err = display_load_atlas(filename, NULL, NULL);

    return err;
}

static int load_map(void) {
    char filename[PATH_MAX];
    snprintf(
        filename, PATH_MAX,
        "%s/map", game_folder
    );
    int err = map_load(filename);

    return err;
}

static int load_sounds(void) {
    char folder[PATH_MAX];
    snprintf(
        folder, PATH_MAX,
        "%s/sfx", game_folder
    );
    int err = sound_load(folder);

    return err;
}
