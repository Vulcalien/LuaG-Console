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

#include "hashtable.h"

#include <SDL2/SDL_mixer.h>

struct Sound {
    Mix_Chunk *chunk;
    u32 channel;
};

static struct Hashtable *sounds_table = NULL;

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

static void destroy_sound(void *sound) {
    Mix_FreeChunk(((struct Sound *) sound)->chunk);
}

void sound_destroy(void) {
    if(sounds_table)
        hashtable_destroy(sounds_table, destroy_sound);

    Mix_CloseAudio();
}

int sound_load(char *sfx_folder) {
    if(sounds_table)
        hashtable_destroy(sounds_table, destroy_sound);
    sounds_table = hashtable_create(512);

    // TODO load sounds
    return 0;
}

int sound_play(const char *name, i32 loops) {
    struct Sound *sound;

    if(hashtable_get(sounds_table, name, (void **) &sound))
        return -1;

    sound->channel = Mix_PlayChannel(-1, sound->chunk, loops);
    if(sound->channel == -1) {
        fprintf(
            stderr,
            "Sound: error playing '%s'\n"
            " - Mix_PlayChannel: %s\n",
            name, Mix_GetError()
        );
    }
    return 0;
}

int sound_stop(const char *name) {
    struct Sound *sound;

    if(hashtable_get(sounds_table, name, (void **) &sound))
        return -1;

    if(sound->channel != -1)
        Mix_HaltChannel(sound->channel);
    return 0;
}

void sound_stop_all(void) {
    Mix_HaltChannel(-1);
}
