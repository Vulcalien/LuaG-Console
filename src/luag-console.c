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
#include "luag-console.h"

#include "gameloop.h"
#include "lua-engine.h"
#include "terminal.h"
#include "input.h"
#include "display.h"

#include <limits.h>

static int init(void);
static void destroy(void);

bool should_quit = false;
char *game_folder = NULL;

static bool should_refresh = false;

int main(int argc, const char *argv[]) {
    int err;

    err = init();
    if(err)
        return err;

    gameloop();

    destroy();
    return 0;
}

void tick(void) {
    input_tick();

    if(!engine_running)
        terminal_tick();

    if(should_quit)
        gameloop_stop();
}

void render(void) {
    if(!should_refresh)
        return;
    should_refresh = false;

    if(!engine_running)
        terminal_render();

    display_refresh();
}

void luag_ask_refresh(void) {
    should_refresh = true;
}

// DEBUG
#include <stdio.h>

static int init(void) {
    game_folder = malloc(PATH_MAX * sizeof(char));

    // DEBUG
    snprintf(game_folder, PATH_MAX, "./console-userdata");

    input_init();

    if(display_init()) {
        display_destroy();
        return -1;
    }

    if(terminal_init()) {
        terminal_destroy();
        return -2;
    }

    input_set_text_mode(true);

    return 0;
}

static void destroy(void) {
    free(game_folder);

    if(engine_running)
        engine_stop();

    display_destroy();
    terminal_destroy();
}
