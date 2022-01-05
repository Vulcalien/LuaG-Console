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
#include "input.h"

#include "shell.h"

#include <SDL2/SDL.h>

static bool text_mode;

void input_init(void) {
    input_set_text_mode(false);
}

void input_tick(void) {
    SDL_Event e;

    while(SDL_PollEvent(&e)) {
        if(e.type == SDL_QUIT) {
            should_quit = true;
            break;
        }

        if(text_mode) {
            if(e.type == SDL_TEXTINPUT && !(SDL_GetModState() & KMOD_CTRL)) {
                shell_receive_input(e.text.text);
            } else if(e.type == SDL_KEYDOWN) {
                if(e.key.keysym.sym == SDLK_RETURN) {
                    shell_receive_input("\n");
                } else if(e.key.keysym.sym == SDLK_BACKSPACE) {
                    shell_receive_input("\b");
                } else if(e.key.keysym.sym == SDLK_DELETE) {
                    shell_receive_input("\x7f");
                } else if(e.key.keysym.sym == SDLK_UP) {
                    shell_receive_input("\x01");
                } else if(e.key.keysym.sym == SDLK_LEFT) {
                    shell_receive_input("\x02");
                } else if(e.key.keysym.sym == SDLK_DOWN) {
                    shell_receive_input("\x03");
                } else if(e.key.keysym.sym == SDLK_RIGHT) {
                    shell_receive_input("\x04");
                }
            }
        } else {
            // not in text mode
            // ...
        }
    }
}

void input_set_text_mode(bool flag) {
    if(flag == text_mode)
        return;
    text_mode = flag;

    if(flag) {
        SDL_StartTextInput();
    } else {
        SDL_StopTextInput();
    }
}
