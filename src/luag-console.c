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
#include "shell.h"
#include "input.h"
#include "display.h"

static int init(void);
static void shutdown(void);

bool should_quit = false;

int main(int argc, const char *argv[]) {
    int init_result = init();
    if(init_result)
        return init_result;

    gameloop();

    return 0;
}

void tick(void) {
    input_tick();

    shell_tick();

    if(should_quit)
        shutdown();
}

void render(void) {
}

static int init(void) {
    input_init();
    if(display_init())
        return -1;
    return 0;
}

static void shutdown(void) {
    gameloop_stop();
}
