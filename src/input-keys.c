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
#include "input-keys.h"

#include "input.h"

#include <stdio.h>
#include <ctype.h>
#include <limits.h>

#include <SDL2/SDL.h>

struct Keymap {
    SDL_Keycode key_code;
    u32 luag_code;
};

static u32 keymaps_size = 0;
static struct Keymap keymaps[8 * KEY_COUNT];

// return nonzero if could not assign
static int assign_key(char *luag_name, char *key_name) {
    // check if keymaps is full
    if(keymaps_size >= sizeof(keymaps) / sizeof(struct Keymap))
        return -1;

    SDL_Keycode key_code = SDL_GetKeyFromName(key_name);
    if(key_code == SDLK_UNKNOWN) {
        fprintf(
            stderr,
            "Input Keys: unrecognized key: '%s'\n"
            " - SDL_GetKeyFromName: %s\n",
            key_name, SDL_GetError()
        );
        return -2;
    }

    struct str_to_code {
        char *str;
        u32 luag_code;
    };
    struct str_to_code str_to_code[KEY_COUNT] = {
        { "up",    KEY_UP    },
        { "left",  KEY_LEFT  },
        { "down",  KEY_DOWN  },
        { "right", KEY_RIGHT },

        { "a", KEY_A },
        { "b", KEY_B },

        { "start",  KEY_START  },
        { "select", KEY_SELECT }
    };

    // make luag_name lowercase
    for(u32 i = 0; luag_name[i] != '\0'; i++)
        luag_name[i] = tolower(luag_name[i]);

    for(u32 i = 0; i < KEY_COUNT; i++) {
        if(strcmp(str_to_code[i].str, luag_name))
            continue;

        keymaps[keymaps_size] = (struct Keymap) {
            .key_code  = key_code,
            .luag_code = str_to_code[i].luag_code
        };
        keymaps_size++;
        return 0;
    }
    return -3;
}

static int parse_file(FILE *file) {
    char line[256];
    while(fgets(line, sizeof(line) / sizeof(char), file)) {
        // luag names are the names given to the keys by luag
        // for example: 'up', 'a', 'select'...
        u32 luag_len = 0;
        char luag_name[8];

        u32 key_len = 0;
        char key_name[64];

        bool reading_name = false;
        for(u32 i = 0; true; i++) {
            char c = line[i];

            if(!reading_name) {
                if(c == ' ' || c == '=' || c == ',' || c == '\n')
                    continue;
                if(c == '\0')
                    break;

                if(luag_len == 0) {
                    // ignore comments
                    if(c == '#')
                        break;

                    luag_name[0] = c;
                    luag_len++;
                } else {
                    key_name[0] = c;
                    key_len++;
                }
                reading_name = true;
            } else {
                if(c == ' ' || c == '=' || c == ',' ||
                   c == '\n' || c == '\0') {
                    if(key_len != 0) {
                        // a key name has been read:
                        // map it to the luag name
                        luag_name[luag_len] = '\0';
                        key_name[key_len] = '\0';

                        if(assign_key(luag_name, key_name))
                            break;

                        key_len = 0;
                    }
                    if(c == '\0')
                        break;

                    reading_name = false;
                    continue;
                }

                if(key_len != 0) {
                    if(key_len >= (sizeof(key_name) / sizeof(char)) - 1)
                        break;

                    key_name[key_len] = c;
                    key_len++;
                } else {
                    // if not reading a key name, then surely it is luag name

                    if(luag_len >= (sizeof(luag_name) / sizeof(char)) - 1)
                        break;

                    luag_name[luag_len] = c;
                    luag_len++;
                }
            }
        }
    }
    return 0;
}

int inputkeys_init(void) {
    int err = 0;

    char filename[PATH_MAX];
    snprintf(filename, PATH_MAX, "%s/keys", config_folder);

    FILE *file = fopen(filename, "r");

    if(file) {
        err = parse_file(file);
        fclose(file);
    } else {
        fprintf(
            stderr,
            "Input Keys: could not load keys file '%s'; "
            "loading defaults instead\n", filename
        );

        snprintf(filename, PATH_MAX, "%s/default_keys", res_folder);

        file = fopen(filename, "r");

        if(!file) {
            fprintf(
                stderr,
                "Input Keys: could not load default keys file '%s'\n",
                filename
            );
            err = -1;
        } else {
            err = parse_file(file);
            fclose(file);
        }
    }
    return err;
}

struct input_Key *inputkeys_get(SDL_Keycode keycode) {
    for(u32 i = 0; i < keymaps_size; i++) {
        if(keymaps[i].key_code == keycode)
            return &input_keys[keymaps[i].luag_code];
    }
    return NULL;
}
