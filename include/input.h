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
#ifndef VULC_LUAG_INPUT
#define VULC_LUAG_INPUT

#include "luag-console.h"

struct input_Key {
    bool is_down;
    bool is_pressed;
    bool is_released;
};

struct input_Btn {
    struct input_Key key;
    struct {
        i32 x;
        i32 y;
    } pos;
};

#define KEY_COUNT (4)

#define KEY_UP    (0)
#define KEY_LEFT  (1)
#define KEY_DOWN  (2)
#define KEY_RIGHT (3)
extern struct input_Key input_keys[KEY_COUNT];

#define BTN_COUNT (3)

#define BTN_LEFT   (0)
#define BTN_MIDDLE (1)
#define BTN_RIGHT  (2)
extern struct input_Btn input_btns[BTN_COUNT];

extern i32 input_scroll;

extern void input_init(void);
extern void input_tick(void);

extern void input_reset_keys(void);
extern void input_set_text_mode(bool flag);

#endif // VULC_LUAG_INPUT
