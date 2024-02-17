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
#ifndef VULC_LUAG_INPUT_KEYS
#define VULC_LUAG_INPUT_KEYS

#include "luag-console.h"

#include "input.h"

#include <SDL.h>

extern int inputkeys_init(void);

extern struct input_Key *inputkeys_get(SDL_Keycode keycode);

#endif // VULC_LUAG_INPUT_KEYS
