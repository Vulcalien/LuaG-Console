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
#ifndef VULC_LUAG_TERMINAL
#define VULC_LUAG_TERMINAL

#include "luag-console.h"

#define TERM_COLOR_NORMAL (0xffffff)
#define TERM_COLOR_ERROR  (0xff0000)
#define TERM_COLOR_INPUT  (0x00ff00)

extern int terminal_init(void);
extern void terminal_tick(void);
extern void terminal_render(void);

extern void terminal_write(const char *text, u32 color);
extern void terminal_receive_input(const char *c);

#endif // VULC_LUAG_TERMINAL
