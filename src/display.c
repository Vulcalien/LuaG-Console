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

static SDL_Surface *atlas_surface = NULL;
static SDL_Texture *atlas_texture = NULL;

static int set_window_icon(void) {
    SDL_Surface *icon = IMG_Load(RESOURCES_DIR "/icon.png");
    if(!icon) {
        fputs("SDL: cannot load window icon\n", stderr);
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
        fputs("SDL: cannot load font file\n", stderr);
        err = -1;
        goto exit;
    }

    // make background transparent
    SDL_SetColorKey(font_surf, SDL_TRUE, SDL_MapRGB(font_surf->format, 0x00, 0x00, 0x00));

    font_texture = SDL_CreateTextureFromSurface(renderer, font_surf);
    if(!font_texture) {
        fputs("SDL: cannot create font texture\n", stderr);
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
        fputs("SDL: could not initialize\n", stderr);
        return -1;
    }

    window = SDL_CreateWindow(
        "LuaG Console - C",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        DISPLAY_WIDTH * 4, DISPLAY_HEIGHT * 4,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN
    );
    if(!window) {
        fputs("SDL: could not create window\n", stderr);
        return -2;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(!renderer) {
        fputs("SDL: could not create renderer\n", stderr);
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

    if(atlas_surface)
        SDL_FreeSurface(atlas_surface);
    if(atlas_texture)
        SDL_DestroyTexture(atlas_texture);

    if(renderer)
        SDL_DestroyRenderer(renderer);
    if(window)
        SDL_DestroyWindow(window);

    IMG_Quit();
    SDL_Quit();
}

static int atlas_update_texture(void) {
    // delete old texture
    if(atlas_texture) {
        SDL_DestroyTexture(atlas_texture);
        atlas_texture = NULL;
    }

    atlas_texture = SDL_CreateTextureFromSurface(renderer, atlas_surface);
    if(!atlas_texture) {
        fputs("SDL: cannot create atlas texture\n", stderr);
        return -1;
    }
    return 0;
}

int display_set_atlas(SDL_Surface *surface) {
    // delete old surface
    if(atlas_surface) {
        SDL_FreeSurface(atlas_surface);
        atlas_surface = NULL;
    }

    if(surface->w != 128 || surface->h != 128) {
        fprintf(
            stderr,
            "SDL: atlas is of wrong size: "
            "(%d, %d) instead of (128, 128)\n",
            surface->w, surface->h
        );
        return -1;
    }

    atlas_surface = surface;
    if(atlas_update_texture())
        return -2;
    return 0;
}

int display_load_atlas(char *filename) {
    SDL_Surface *surface = IMG_Load(filename);
    if(!surface) {
        fprintf(stderr, "SDL: cannot load atlas file %s\n", filename);
        return -1;
    }

    return display_set_atlas(surface);
}

void display_atlas_set_color_key(u32 color, bool active_flag) {
    SDL_SetColorKey(
        atlas_surface, active_flag,
        SDL_MapRGB(atlas_surface->format, color >> 16, color >> 8, color)
    );

    atlas_update_texture();
}

void display_refresh(void) {
    SDL_RenderPresent(renderer);

    SDL_SetRenderDrawColor(renderer, 0x40, 0x40, 0x40, 0xff);
    SDL_RenderClear(renderer);
}

void display_clear(u32 color) {
    display_fill(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, color);
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

void display_draw_from_atlas(u32 id,    u32 x,       u32 y,
                             u32 scale, u32 sw,      u32 sh,
                             u32 rot,   bool h_flip, bool v_flip,
                             u32 color_mod) {
    struct SDL_Rect src = {
        .x = (id % 16) * 8, .y = (id / 16) * 8,
        .w = sw * 8,        .h = sh * 8
    };

    struct SDL_Rect dst = {
        .x = x,      .y = y,
        .w = sw * scale * 8, .h = sh * scale * 8
    };

    SDL_SetTextureColorMod(
        atlas_texture,
        color_mod >> 16, color_mod >> 8, color_mod
    );

    if(rot % 4 == 0 && !h_flip && !v_flip) {
        SDL_RenderCopy(
            renderer, atlas_texture,
            &src, &dst
        );
    } else {
        SDL_RendererFlip flip = SDL_FLIP_NONE;
        if(h_flip)
            flip |= SDL_FLIP_HORIZONTAL;
        if(v_flip)
            flip |= SDL_FLIP_VERTICAL;

        SDL_RenderCopyEx(
            renderer, atlas_texture,
            &src, &dst,
            90 * rot, NULL,
            flip
        );
    }
}
