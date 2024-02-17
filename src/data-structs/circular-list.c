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
#include "data-structs/circular-list.h"

// Implementation of a Circular List of generic items. The list grows
// right-to-left and overwrites the elements as it grows.

struct CircularList {
    u32 size;
    void **values;

    u32 count;
    i32 head;
};

struct CircularList *circularlist_create(u32 size) {
    struct CircularList *result = malloc(sizeof(struct CircularList));

    *result = (struct CircularList) {
        .size = size,
        .values = calloc(size, sizeof(void *)),

        .count = 0,
        .head = size - 1
    };

    return result;
}

void circularlist_destroy(struct CircularList *list,
                          void (*destroy_value_fn)(void *)) {
    if(!list)
        return;

    if(destroy_value_fn) {
        for(u32 i = 0; i < list->size; i++) {
            void *value = list->values[i];
            if(value)
                destroy_value_fn(value);
        }
    }

    free(list->values);
    free(list);
}

void circularlist_add(struct CircularList *list, void *value,
                      void (*destroy_value_fn)(void *)) {
    if(destroy_value_fn) {
        // destroy the element in that position, if present
        void *old = list->values[list->head];
        if(old)
            destroy_value_fn(old);
    }

    if(list->count < list->size)
        list->count++;

    list->values[list->head] = value;
    list->head--;
    if(list->head < 0)
        list->head = list->size - 1;
}

void *circularlist_get(struct CircularList *list, u32 index) {
    if(index >= list->count)
        return NULL;

    return list->values[(list->head + 1 + index) % (list->size)];
}

u32 circularlist_count(struct CircularList *list) {
    return list->count;
}

// The elements inside the list are not leaked: they will be destroyed
// either when a new one overwrites it or when the list is destroyed.
void circularlist_clear(struct CircularList *list) {
    list->count = 0;
}
