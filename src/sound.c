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

#include <string.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include <SDL2/SDL_mixer.h>

struct Sound {
    Mix_Chunk *chunk;
    i32 channel;
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

static void load_sounds(DIR *dir, char *folder_path, u32 root_index) {
    struct dirent *file;

    while((file = readdir(dir))) {
        if(!strcmp(file->d_name, ".") || !strcmp(file->d_name, ".."))
            continue;

        char *filename = malloc(PATH_MAX * sizeof(char));
        snprintf(filename, PATH_MAX, "%s/%s", folder_path, file->d_name);

        struct stat st;
        if(stat(filename, &st)) {
            fprintf(
                stderr,
                "Sound: could not retrieve stat for file '%s'\n",
                filename
            );
            goto while_end;
        }

        if(S_ISDIR(st.st_mode)) {
            DIR *subdir = opendir(filename);
            if(!subdir) {
                fprintf(
                    stderr,
                    "Sound: could not open folder '%s'\n",
                    filename
                );
                goto while_end;
            }

            load_sounds(subdir, filename, root_index);
        } else {
            // relative to console-userdata/sfx
            char *short_name = filename + root_index;

            // check if the filename ends with '.wav'
            u32 str_len = strlen(short_name);
            if(strcmp(short_name + str_len - 4, ".wav"))
                goto while_end;

            Mix_Chunk *chunk = Mix_LoadWAV(filename);
            if(!chunk) {
                fprintf(
                    stderr,
                    "Sound: could not read file '%s'\n"
                    " - Mix_LoadWAV: %s\n",
                    filename, Mix_GetError()
                );
                goto while_end;
            }

            // copy the name without the '.wav' suffix
            char *sound_name = calloc(str_len - 4 + 1, sizeof(char));
            strncpy(sound_name, short_name, str_len - 4);

            printf("Loading sound: %s\n", sound_name);

            struct Sound *sound = malloc(sizeof(struct Sound));
            *sound = (struct Sound) {
                .chunk = chunk,
                .channel = -1
            };
            hashtable_set(sounds_table, sound_name, sound);
        }

        while_end:
        free(filename);
    }
}

int sound_load(char *sfx_folder) {
    if(sounds_table)
        hashtable_destroy(sounds_table, destroy_sound);
    sounds_table = hashtable_create(512);

    DIR *dir = opendir(sfx_folder);
    if(!dir) {
        fputs("Sound: could not find sound folder\n", stderr);
        return -1;
    }
    load_sounds(dir, sfx_folder, strlen(sfx_folder) + 1);
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
