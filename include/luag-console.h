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
#ifndef VULC_LUAG_CORE
#define VULC_LUAG_CORE

#include <stdlib.h>

#include <vulcalien/vulcalien.h>

#define LUAG_VERSION "C Port - 0.0.1-WIP"
#define COPYRIGHT_NOTICE "Copyright 2022 Vulcalien"

// TODO this is OS-dependant
#define RESOURCES_DIR "/usr/share/luag-console"

#define TPS (60)

extern bool should_quit;

extern void tick(void);
extern void render(void);

extern void luag_ask_refresh(void);

#endif // VULC_LUAG_CORE
