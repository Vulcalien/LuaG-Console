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
#include "input.h"

#include "lua-engine.h"
#include "terminal.h"

#include <SDL2/SDL.h>

static bool text_mode;

struct input_Key input_keys[KEY_COUNT + BTN_COUNT];
struct input_Mouse input_mouse;

void input_init(void) {
    input_reset_keys();

    input_set_text_mode(false);
}

void input_tick(void) {
    // update is_down based on release_count
    // (release_count only affects is_down one tick *after* the event occurred)
    for(u32 i = 0; i < KEY_COUNT + BTN_COUNT; i++) {
        struct input_Key *key = &input_keys[i];

        if(key->release_count)
            key->is_down = false;

        key->press_count   = 0;
        key->release_count = 0;
    }

    input_mouse.scroll = 0;

    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        if(e.type == SDL_QUIT) {
            should_quit = true;
            break;
        }

        if(engine_running) {
            if(e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
                struct input_Key *key = NULL;
                switch(e.key.keysym.sym) {
                    // ctrl+F4 or ctrl+F8: stop
                    case SDLK_F4:
                    case SDLK_F8:
                        if(e.key.keysym.mod & KMOD_CTRL) {
                            engine_stop();
                            return;
                        }
                        break;

                    case SDLK_UP:
                    case SDLK_w:
                        key = &input_keys[KEY_UP];
                        break;
                    case SDLK_LEFT:
                    case SDLK_a:
                        key = &input_keys[KEY_LEFT];
                        break;
                    case SDLK_DOWN:
                    case SDLK_s:
                        key = &input_keys[KEY_DOWN];
                        break;
                    case SDLK_RIGHT:
                    case SDLK_d:
                        key = &input_keys[KEY_RIGHT];
                        break;
                }
                if(key) {
                    switch(e.type) {
                        case SDL_KEYDOWN:
                            if(!e.key.repeat)
                                key->press_count++;
                            break;
                        case SDL_KEYUP:
                            key->release_count++;
                            break;
                    }
                }
            } else if(e.type == SDL_MOUSEBUTTONDOWN ||
                      e.type == SDL_MOUSEBUTTONUP) {
                struct input_Key *btn = NULL;
                switch(e.button.button) {
                    case SDL_BUTTON_LEFT:
                        btn = &input_btns[BTN_LEFT];
                        break;
                    case SDL_BUTTON_MIDDLE:
                        btn = &input_btns[BTN_MIDDLE];
                        break;
                    case SDL_BUTTON_RIGHT:
                        btn = &input_btns[BTN_RIGHT];
                        break;
                }
                if(btn) {
                    if(e.type == SDL_MOUSEBUTTONDOWN)
                        btn->press_count++;
                    else
                        btn->release_count++;
                }
            } else if(e.type == SDL_MOUSEWHEEL) {
                input_mouse.scroll += -e.wheel.y;
            } else if(e.type == SDL_MOUSEMOTION) {
                input_mouse.x = e.motion.x;
                input_mouse.y = e.motion.y;
            }
        } else {
            // engine is not running: send input to terminal
            if(e.type == SDL_TEXTINPUT && !(SDL_GetModState() & KMOD_CTRL)) {
                terminal_receive_input(e.text.text);
            } else if(e.type == SDL_KEYDOWN) {
                if(e.key.keysym.sym == SDLK_RETURN) {
                    terminal_receive_input("\n");
                } else if(e.key.keysym.sym == SDLK_BACKSPACE) {
                    terminal_receive_input("\b");
                } else if(e.key.keysym.sym == SDLK_DELETE) {
                    terminal_receive_input("\x7f");
                } else if(e.key.keysym.sym == SDLK_UP) {
                    terminal_receive_input("\x11");
                } else if(e.key.keysym.sym == SDLK_LEFT) {
                    terminal_receive_input("\x12");
                } else if(e.key.keysym.sym == SDLK_DOWN) {
                    terminal_receive_input("\x13");
                } else if(e.key.keysym.sym == SDLK_RIGHT) {
                    terminal_receive_input("\x14");
                }
            } else if(e.type == SDL_MOUSEWHEEL) {
                terminal_scroll(-e.wheel.y);
            }
        }
    }

    // update is_down based on press_count
    for(u32 i = 0; i < KEY_COUNT + BTN_COUNT; i++) {
        struct input_Key *key = &input_keys[i];

        if(key->press_count)
            key->is_down = true;
    }
}

void input_reset_keys(void) {
    for(u32 i = 0; i < KEY_COUNT + BTN_COUNT; i++) {
        input_keys[i] = (struct input_Key) {
            .is_down       = false,
            .press_count   = 0,
            .release_count = 0
        };
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
