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
#ifndef VULC_LUAG_DISPLAY
#define VULC_LUAG_DISPLAY

#include "luag-console.h"

#include <SDL2/SDL.h>

#define DISPLAY_WIDTH  (160)
#define DISPLAY_HEIGHT (160)

#define SPRITE_SIZE    (8)

#define ATLAS_WIDTH    (SPRITE_SIZE * 16)
#define ATLAS_HEIGHT   (SPRITE_SIZE * 16)

#define CHAR_WIDTH     (5)
#define CHAR_HEIGHT    (8)
#define LINE_SPACING   (1)
#define LETTER_SPACING (1)

extern int display_init(void);
extern void display_destroy(void);

extern int display_load_atlas(char *filename,
                              SDL_Surface **surface, SDL_Texture **texture);
extern int display_update_atlas(SDL_Surface **surface, SDL_Texture **texture);

extern void display_atlas_set_color_key(u32 color, bool active_flag);

extern void display_refresh(void);

extern void display_clear(u32 color);
extern void display_fill(u32 x, u32 y, u32 w, u32 h, u32 color, u8 alpha);
extern void display_write(const char *text, u32 color, u8 alpha,
                          i32 x, i32 y);
extern void display_draw_from_atlas(SDL_Texture *texture,
                                    u32 id,    u32 x,       u32 y,
                                    u32 scale, u32 sw,      u32 sh,
                                    u32 rot,   bool h_flip, bool v_flip,
                                    u8  alpha, u32 col_mod);

#endif // VULC_LUAG_DISPLAY
