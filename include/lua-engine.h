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
#ifndef VULC_LUAG_LUA_ENGINE
#define VULC_LUAG_LUA_ENGINE

#include "luag-console.h"

extern bool engine_running;

extern void engine_load(bool is_editor);
extern void engine_reload(void);
extern void engine_stop(void);

extern void engine_ask_exit(i8 code, const char *msg);

extern void engine_tick(void);
extern void engine_render(void);

#endif // VULC_LUAG_LUA_ENGINE
