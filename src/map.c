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
#include "map.h"

#include <stdio.h>

static int fread_u32_big_endian(u32 *result, FILE *file);

struct Map map = {
    .width  = 0,
    .height = 0,
    .tiles = NULL
};

int map_init(void) {
    return 0;
}

void map_destroy(void) {
    if(map.tiles)
        free(map.tiles);
}

int map_load(char *filename) {
    int err = 0;

    if(map.tiles) {
        free(map.tiles);
        map.tiles = NULL;
    }

    FILE *file = fopen(filename, "rb");

    if(!file) {
        fprintf(stderr, "Map: could not load map file '%s'\n", filename);
        err = -1;
        goto exit;
    }

    if(fread_u32_big_endian(&map.width, file) ||
       fread_u32_big_endian(&map.height, file)) {
        goto invalid_file;
    }

    u32 map_size = map.width * map.height;

    map.tiles = malloc(map_size * sizeof(u8));
    if(fread(map.tiles, sizeof(u8), map_size, file) < map_size)
        goto invalid_file;

    // skip the "invalid file" error
    goto exit;

    invalid_file:
    fputs("Map: map file is invalid\n", stderr);

    exit:
    if(file)
        fclose(file);

    return err;
}

static int fread_u32_big_endian(u32 *result, FILE *file) {
    u8 b[4];

    if(fread(b, sizeof(u8), 4, file) < 4)
        return -1;

    *result = b[0] << 24 | b[1] << 16 | b[2] << 8 | b[3];

    return 0;
}
