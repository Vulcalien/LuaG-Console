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
#ifndef VULC_LUAG_CORE
#define VULC_LUAG_CORE

#include <stdlib.h>

#include <vulcalien/vulcalien.h>

#define LUAG_VERSION "C Port - 0.0.1-WIP"
#define COPYRIGHT_NOTICE "Copyright 2022 Vulcalien"

#define USERDATA_FOLDER "console-userdata"

// TODO delete
#define RESOURCES_DIR "/usr/share/luag-console"

#define TPS (60)

extern bool should_quit;

extern bool dev_mode;

extern char *res_folder;
extern char *game_folder;

extern void tick(void);
extern void render(void);

#endif // VULC_LUAG_CORE
