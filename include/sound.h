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
#ifndef VULC_LUAG_SOUND
#define VULC_LUAG_SOUND

#include "luag-console.h"

// call init after initializing the display
extern int sound_init(void);

// call destroy before destroying the display
extern void sound_destroy(void);

extern void sound_play(char *name, i32 loops);
extern void sound_stop(char *name);

extern void sound_stop_all(void);

#endif // VULC_LUAG_SOUND
