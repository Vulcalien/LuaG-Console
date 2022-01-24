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

#include <stdio.h>
#include <limits.h>

static int load_cartridge_info(void);
static int load_atlas(void);
static int load_map(void);

struct cartridge_Info cartridge_info;

static char *cartridge_folder;

int cartridge_init(void) {
    return 0;
}

void cartridge_destroy(void) {
    free(cartridge_folder);
}

char *cartridge_extract(const char *filename) {
    // TODO
    return NULL;
}

int cartridge_load_files(void) {
    if(load_cartridge_info())
        return -1;
    if(load_atlas())
        return -2;
    if(load_map())
        return -3;
    return 0;
}

static int load_cartridge_info(void) {
    char *filename = malloc(PATH_MAX * sizeof(char));
    snprintf(
        filename, PATH_MAX,
        "%s/cartridge-info", game_folder
    );
    FILE *file = fopen(filename, "r");
    free(filename);

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
    char *filename = malloc(PATH_MAX * sizeof(char));
    snprintf(
        filename, PATH_MAX,
        "%s/atlas.png", game_folder
    );
    int err = display_load_atlas(filename);
    free(filename);

    return err;
}

static int load_map(void) {
    char *filename = malloc(PATH_MAX * sizeof(char));
    snprintf(
        filename, PATH_MAX,
        "%s/map", game_folder
    );
    int err = map_load(filename);
    free(filename);

    return err;
}
