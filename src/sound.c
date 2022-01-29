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
#include "sound.h"

#include <SDL2/SDL_mixer.h>

int sound_init(void) {
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048)) {
        fprintf(
            stderr,
            "Sound: could not initialize\n"
            " - Mix_OpenAudio: %s\n", Mix_GetError()
        );
        return -1;
    }
    return 0;
}

void sound_destroy(void) {
    Mix_CloseAudio();
}

void sound_play(char *name, i32 loops) {
    // TODO sound
}

void sound_stop(char *name) {
    // TODO sound
}

void sound_stop_all(void) {
    Mix_HaltChannel(-1);
}
