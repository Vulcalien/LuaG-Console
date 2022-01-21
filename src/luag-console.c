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
#include "luag-console.h"

#include "gameloop.h"
#include "lua-engine.h"
#include "terminal.h"
#include "input.h"
#include "display.h"
#include "cartridge.h"

#include <limits.h>

static int init(void);
static void destroy(void);

bool should_quit = false;

bool dev_mode = false;
char *game_folder = NULL;

int main(int argc, const char *argv[]) {
    int err = 0;

    err = init();
    if(err)
        goto exit;

    gameloop();

    exit:
    destroy();
    return err;
}

void tick(void) {
    input_tick();

    if(engine_running)
        engine_tick();
    else
        terminal_tick();

    if(should_quit)
        gameloop_stop();
}

void render(void) {
    if(!engine_running)
        terminal_render();

    display_refresh();
}

static int init(void) {
    input_init();

    if(display_init())
        return -1;
    if(terminal_init())
        return -2;
    if(cartridge_init())
        return -3;

    input_set_text_mode(true);

    return 0;
}

static void destroy(void) {
    if(engine_running)
        engine_stop();

    display_destroy();
    terminal_destroy();
    cartridge_destroy();
}
