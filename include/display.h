/* Copyright 2022 Vulcalien
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 only.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef VULC_LUAG_DISPLAY
#define VULC_LUAG_DISPLAY

#include "luag-console.h"

#define DISPLAY_WIDTH  160
#define DISPLAY_HEIGHT 160

extern int display_init(void);
extern void display_refresh(void);

// TODO deallocate memory on exit

extern void display_clear(u32 color);
extern void display_write(const char *text, u32 color, i32 x, i32 y);

#endif // VULC_LUAG_DISPLAY
