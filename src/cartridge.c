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

#include <stdio.h>
#include <limits.h>

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

static inline FILE *open_cartridge_info_file(void) {
    char *filename = malloc(PATH_MAX * sizeof(char));
    snprintf(
        filename, PATH_MAX,
        "%s/cartridge-info", game_folder
    );

    FILE *file = fopen(filename, "r");
    free(filename);

    return file;
}

int cartridge_load_info(void) {
    FILE *file = open_cartridge_info_file();
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
            "is invalid\n",
            true
        );
        err = -1;
    }

    fclose(file);
    return err;
}
