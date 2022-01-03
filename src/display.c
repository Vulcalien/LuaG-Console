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
#include "display.h"

#include "shell.h"

#include <stdio.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

static SDL_Window *window;
static SDL_Renderer *renderer;

static bool text_mode;

int display_init(void) {
    if(SDL_Init(SDL_INIT_VIDEO)) {
        fputs("SDL display: could not initialize", stderr);
        return -1;
    }

    window = SDL_CreateWindow(
        "LuaG Console - C",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        DISPLAY_WIDTH, DISPLAY_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if(!window) {
        fputs("SDL display: could not create window", stderr);
        return -2;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!renderer) {
        fputs("SDL display: could not create renderer", stderr);
        return -3;
    }

    IMG_Init(IMG_INIT_PNG);

    // set window icon
    SDL_Surface *icon = IMG_Load("res/icon.png");
    SDL_SetWindowIcon(window, icon);
    SDL_FreeSurface(icon);

    display_set_text_mode(true);

    return 0;
}

void display_tick(void) {
    SDL_Event e;

    while(SDL_PollEvent(&e)) {
        if(e.type == SDL_QUIT) {
            should_quit = true;
            break;
        }

        if(text_mode) {
            if(e.type == SDL_TEXTINPUT) {
                if(SDL_GetModState() & KMOD_CTRL) {
                    // TODO ctrl+w, ctrl+c, ctrl+alt+c/v
                } else if(SDL_GetModState() & KMOD_ALT) {
                    // pass
                } else {
                    shell_receive_input(e.text.text);
                }
            } else if(e.type == SDL_KEYDOWN) {
            }
        } else {
            // not in text mode
            // ...
        }
    }
}

void display_set_text_mode(bool flag) {
    if(flag == text_mode)
        return;
    text_mode = flag;

    if(flag) {
        SDL_StartTextInput();
    } else {
        SDL_StopTextInput();
    }
}
