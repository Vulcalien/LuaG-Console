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
#ifndef VULC_LUAG_SHELL
#define VULC_LUAG_SHELL

#include "luag-console.h"

#define SHELL_COLOR_NORMAL (0xffffff)
#define SHELL_COLOR_ERROR  (0xff0000)
#define SHELL_COLOR_INPUT  (0x00ff00)

extern int shell_init(void);
extern void shell_tick(void);
extern void shell_render(void);

extern void shell_write(const char *text, u32 color);
extern void shell_receive_input(const char *c);

#endif // VULC_LUAG_SHELL
