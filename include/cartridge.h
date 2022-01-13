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
#ifndef VULC_LUAG_CARTRIDGE
#define VULC_LUAG_CARTRIDGE

#include "luag-console.h"

extern int cartridge_init(void);
extern void cartridge_destroy(void);

// returns the folder the cartridge was extracted into
// or NULL if the cartridge could not be found
extern char *cartridge_extract(const char *filename);

#endif // VULC_LUAG_CARTRIDGE
