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
#include "gameloop.h"

#include <SDL2/SDL.h>

static bool text_mode;
#define TEXT_MODE_BUFFER (128)
static char text_mode_text[TEXT_MODE_BUFFER];

struct input_Key input_keys[KEY_COUNT + BTN_COUNT];
struct input_Mouse input_mouse;

void input_init(void) {
    input_set_text_mode(true);
}

void input_tick(void) {
    if(engine_running) {
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

        text_mode_text[0] = '\0';
    }

    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        if(e.type == SDL_QUIT) {
            should_quit = true;
            break;
        }

        if(e.type == SDL_KEYDOWN &&
           !e.key.repeat &&
           e.key.keysym.mod & KMOD_CTRL &&
           e.key.keysym.sym == SDLK_F3) {
            gameloop_print_performance = !gameloop_print_performance;
        }

        if(engine_running) {
            // check for stop and restart
            if(e.type == SDL_KEYDOWN &&
               e.key.keysym.mod & KMOD_CTRL &&
               !e.key.repeat) {
                switch(e.key.keysym.sym) {
                    // ctrl+F4 or ctrl+F8: stop
                    case SDLK_F4:
                    case SDLK_F8:
                        engine_stop();
                        return;
                    // ctrl+F5 or ctrl+F7: restart
                    case SDLK_F5:
                    case SDLK_F7:
                        engine_reload();
                        return;
                }
            }

            if(e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
                struct input_Key *key = NULL;
                // TODO find a way to let the end user choose these
                switch(e.key.keysym.sym) {
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
                    case SDLK_p:
                    case SDLK_l:
                    case SDLK_m:
                        key = &input_keys[KEY_A];
                        break;
                    case SDLK_o:
                    case SDLK_k:
                    case SDLK_n:
                        key = &input_keys[KEY_B];
                        break;
                    case SDLK_RETURN:
                        key = &input_keys[KEY_START];
                        break;
                    case SDLK_BACKSPACE:
                        key = &input_keys[KEY_SELECT];
                        break;
                }
                if(key) { // TODO && !text_mode ???
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

                if(text_mode && e.type == SDL_KEYDOWN) {
                    // add '\n' and '\b' to the input string
                    char *str = NULL;
                    switch(e.key.keysym.sym) {
                        case SDLK_RETURN:
                            str = "\n";
                            break;
                        case SDLK_BACKSPACE:
                            str = "\b";
                            break;
                    }

                    if(str) {
                        u32 len = strlen(text_mode_text);
                        strncpy(
                            text_mode_text + len, str,
                            TEXT_MODE_BUFFER - len
                        );
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
            } else if(e.type == SDL_TEXTINPUT) {
                if(text_mode) {
                    u32 len = strlen(text_mode_text);
                    strncpy(
                        text_mode_text + len, e.text.text,
                        TEXT_MODE_BUFFER - len
                    );
                }
            }
        } else {
            // engine is not running: send input to terminal
            if(e.type == SDL_TEXTINPUT && !(SDL_GetModState() & KMOD_CTRL)) {
                terminal_receive_input(e.text.text);
            } else if(e.type == SDL_KEYDOWN) {
                char *str = NULL;
                switch (e.key.keysym.sym) {
                    case SDLK_RETURN:
                        str = "\n";
                        break;
                    case SDLK_BACKSPACE:
                        str = "\b";
                        break;
                    case SDLK_DELETE:
                        str = "\x7f";
                        break;
                    case SDLK_UP:
                        str = "\x11";
                        break;
                    case SDLK_LEFT:
                        str = "\x12";
                        break;
                    case SDLK_DOWN:
                        str = "\x13";
                        break;
                    case SDLK_RIGHT:
                        str = "\x14";
                        break;
                }
                if(str)
                    terminal_receive_input(str);
            } else if(e.type == SDL_MOUSEWHEEL) {
                terminal_scroll(-e.wheel.y);
            }
        }
    }

    if(engine_running) {
        // update is_down based on press_count
        for(u32 i = 0; i < KEY_COUNT + BTN_COUNT; i++) {
            struct input_Key *key = &input_keys[i];

            if(key->press_count)
                key->is_down = true;
        }
    }
}

void input_reset(void) {
    // reset all keys
    for(u32 i = 0; i < KEY_COUNT + BTN_COUNT; i++) {
        input_keys[i] = (struct input_Key) {
            .is_down       = false,
            .press_count   = 0,
            .release_count = 0
        };
    }

    input_set_text_mode(false);
}

void input_set_text_mode(bool flag) {
    if(flag)
        SDL_StartTextInput();
    else
        SDL_StopTextInput();

    text_mode = flag;
}

char *input_get_text(void) {
    return text_mode_text;
}
