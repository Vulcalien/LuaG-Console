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
#ifndef VULC_LUAG_TERMINAL
#define VULC_LUAG_TERMINAL

#include "luag-console.h"

extern int terminal_init(void);
extern void terminal_destroy(void);

extern void terminal_tick(void);
extern void terminal_render(void);

extern void terminal_receive_input(const char *c);

extern void terminal_clear(void);
extern void terminal_write(const char *text, bool is_error);

#endif // VULC_LUAG_TERMINAL
