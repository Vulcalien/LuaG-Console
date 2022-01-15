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
#include "display.h"

#include <stdio.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

static SDL_Window *window;
static SDL_Renderer *renderer;

static SDL_Texture *font_texture;
static SDL_Texture *atlas_texture = NULL;

static int set_window_icon(void) {
    SDL_Surface *icon = IMG_Load(RESOURCES_DIR "/icon.png");
    if(!icon) {
        fputs("SDL display: cannot load window icon\n", stderr);
        return -1;
    }

    SDL_SetWindowIcon(window, icon);
    SDL_FreeSurface(icon);
    return 0;
}

static int load_font(void) {
    int err = 0;

    SDL_Surface *font_surf = IMG_Load(RESOURCES_DIR "/font.png");
    if(!font_surf) {
        fputs("SDL display: cannot load font file\n", stderr);
        err = -1;
        goto exit;
    }

    // make background transparent
    SDL_SetColorKey(font_surf, SDL_TRUE, SDL_MapRGB(font_surf->format, 0x00, 0x00, 0x00));

    font_texture = SDL_CreateTextureFromSurface(renderer, font_surf);
    if(!font_texture) {
        fputs("SDL display: cannot create font texture\n", stderr);
        err = -2;
        goto exit;
    }

    exit:
    if(font_surf)
        SDL_FreeSurface(font_surf);

    return err;
}

int display_init(void) {
    if(SDL_Init(SDL_INIT_VIDEO)) {
        fputs("SDL display: could not initialize\n", stderr);
        return -1;
    }

    window = SDL_CreateWindow(
        "LuaG Console - C",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        DISPLAY_WIDTH * 4, DISPLAY_HEIGHT * 4,
        SDL_WINDOW_SHOWN
    );
    if(!window) {
        fputs("SDL display: could not create window\n", stderr);
        return -2;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!renderer) {
        fputs("SDL display: could not create renderer\n", stderr);
        return -3;
    }
    SDL_RenderSetLogicalSize(renderer, DISPLAY_WIDTH, DISPLAY_HEIGHT);

    IMG_Init(IMG_INIT_PNG);

    if(set_window_icon())
        return -4;
    if(load_font())
        return -5;

    return 0;
}

void display_destroy(void) {
    if(font_texture)
        SDL_DestroyTexture(font_texture);
    if(atlas_texture)
        SDL_DestroyTexture(atlas_texture);

    if(renderer)
        SDL_DestroyRenderer(renderer);
    if(window)
        SDL_DestroyWindow(window);

    IMG_Quit();
    SDL_Quit();
}

int display_set_atlas(char *filename) {
    int err = 0;

    SDL_Surface *atlas_surf = IMG_Load(filename);
    if(!atlas_surf) {
        fprintf(
            stderr,
            "SDL: cannot load atlas file %s\n", filename
        );
        err = -1;
        goto exit;
    }

    atlas_texture = SDL_CreateTextureFromSurface(renderer, atlas_surf);
    if(!atlas_texture) {
        fputs("SDL: cannot create atlas texture\n", stderr);
        err = -2;
        goto exit;
    }

    exit:
    if(atlas_surf)
        SDL_FreeSurface(atlas_surf);

    return err;
}

void display_refresh(void) {
    SDL_RenderPresent(renderer);
}

void display_clear(u32 color) {
    SDL_SetRenderDrawColor(renderer, color >> 16, color >> 8, color, 0xff);
    SDL_RenderClear(renderer);
}

static void display_draw_char(char c, u32 color, i32 x, i32 y) {
    if(c < ' ' || c > '~')
        return;

    SDL_Rect src = {
        .x = (c - 32) * (CHAR_WIDTH + 1), .y = 0,
        .w = CHAR_WIDTH,                  .h = CHAR_HEIGHT
    };

    SDL_Rect dst = {
        .x = x,          .y = y,
        .w = CHAR_WIDTH, .h = CHAR_HEIGHT
    };

    SDL_SetTextureColorMod(font_texture, color >> 16, color >> 8, color);
    SDL_RenderCopy(renderer, font_texture, &src, &dst);
}

void display_fill(u32 x, u32 y, u32 w, u32 h, u32 color) {
    SDL_SetRenderDrawColor(renderer, color >> 16, color >> 8, color, 0xff);
    SDL_Rect rect = { x, y, w, h };
    SDL_RenderFillRect(renderer, &rect);
}

void display_write(const char *text, u32 color, i32 x, i32 y) {
    u32 len = strlen(text);

    i32 xdraw = x;
    i32 ydraw = y;

    for(u32 i = 0; i < len; i++) {
        char c = text[i];

        if(c == '\n') {
            xdraw = x;
            ydraw += CHAR_HEIGHT + LINE_SPACING;
        } else {
            display_draw_char(c, color, xdraw, ydraw);

            xdraw += CHAR_WIDTH + LETTER_SPACING;
        }
    }
}
