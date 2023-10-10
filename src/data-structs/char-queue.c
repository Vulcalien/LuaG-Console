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
#include "data-structs/char-queue.h"

// Implementation of a Circular Queue of characters using an array

struct CharQueue {
    u32 size;
    char *array;

    u32 count;

    i32 tail;
    i32 head;
};

struct CharQueue *charqueue_create(u32 size) {
    struct CharQueue *queue = malloc(sizeof(struct CharQueue));

    *queue = (struct CharQueue) {
        .size = size,
        .array = calloc(size, sizeof(char)),

        .count = 0,

        .tail = 0,
        .head = 0
    };

    return queue;
}

void charqueue_destroy(struct CharQueue *queue) {
    free(queue->array);
    free(queue);
}

bool charqueue_is_empty(struct CharQueue *queue) {
    return queue->count == 0;
}

bool charqueue_is_full(struct CharQueue *queue) {
    return queue->count == queue->size;
}

bool charqueue_enqueue(struct CharQueue *queue, char c) {
    if(charqueue_is_full(queue))
        return false;

    queue->array[queue->head] = c;
    queue->head = (queue->head + 1) % queue->size;
    queue->count++;

    return true;
}

char charqueue_dequeue(struct CharQueue *queue) {
    if(charqueue_is_empty(queue))
        return '\0';

    char c = queue->array[queue->tail];
    queue->tail = (queue->tail + 1) % queue->size;
    queue->count--;

    return c;
}
