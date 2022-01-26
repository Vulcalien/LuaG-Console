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
#ifndef VULC_LUAG_MAP
#define VULC_LUAG_MAP

#include "luag-console.h"

struct Map {
    u32 width;
    u32 height;
    u8 *tiles;
};

extern struct Map map;

extern int map_init(void);
extern void map_destroy(void);

extern int map_load(char *filename);

#define map_get_tile(x, y)\
    map.tiles[x + y * map.width]

#define map_set_tile(x, y, tile)\
    do{\
        map.tiles[x + y * map.width] = tile;\
    }while(0)

#endif // VULC_LUAG_MAP
