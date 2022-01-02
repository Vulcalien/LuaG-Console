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
#include "gameloop.h"

#include <SDL2/SDL.h>

#ifdef PERFORMANCE_THREAD
    #include <stdio.h>
    #include <pthread.h>

    static u32 counter_ticks = 0;
    static u32 counter_frames = 0;

    static pthread_t performance_thread;

    static void *tps_counter(void *arg) {
        while(true) {
            printf("tps: %d - fps: %d", counter_ticks, counter_frames);

            counter_ticks = 0;
            counter_frames = 0;

            SDL_Delay(1000);
        }
        return NULL;
    }
#endif

static bool running = false;

void gameloop(void) {
    #ifdef PERFORMANCE_THREAD
        pthread_create(&performance_thread, NULL, tps_counter, NULL);
    #endif

    const u64 time_in_second = SDL_GetPerformanceFrequency();
    const u64 time_per_tick = time_in_second / TPS;

    u64 last_time = SDL_GetPerformanceCounter();
    u64 unprocessed_time = time_per_tick;

    running = true;
    while(running) {
        u64 now = SDL_GetPerformanceCounter();
        i64 passed_time = now - last_time;
        last_time = now;

        if(passed_time < 0)
            passed_time = 0;
        else if(passed_time >= time_in_second)
            passed_time = time_in_second;

        unprocessed_time += passed_time;

        bool ticked = false;
        while(unprocessed_time >= time_per_tick) {
            unprocessed_time -= time_per_tick;

            tick();
            ticked = true;

            #ifdef PERFORMANCE_THREAD
                counter_ticks++;
            #endif
        }

        if(ticked) {
            render();

            #ifdef PERFORMANCE_THREAD
                counter_frames++;
            #endif
        }

        SDL_Delay(4);
    }
}

void gameloop_stop(void) {
    running = false;

    #ifdef PERFORMANCE_THREAD
        pthread_cancel(performance_thread);
    #endif
}
