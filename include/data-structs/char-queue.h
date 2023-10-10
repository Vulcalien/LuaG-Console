/* Copyright 2023 Vulcalien
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
#ifndef VULC_LUAG_CHARQUEUE
#define VULC_LUAG_CHARQUEUE

#include "luag-console.h"

struct CharQueue;

extern struct CharQueue *charqueue_create(u32 size);

extern void charqueue_destroy(struct CharQueue *queue);

extern bool charqueue_is_empty(struct CharQueue *queue);
extern bool charqueue_is_full(struct CharQueue *queue);

// returns false if the queue is full
extern bool charqueue_enqueue(struct CharQueue *queue, char c);

// returns '\0' if the queue is empty
extern char charqueue_dequeue(struct CharQueue *queue);

#endif // VULC_LUAG_CHARQUEUE
