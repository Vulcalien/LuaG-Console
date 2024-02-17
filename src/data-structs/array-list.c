/* Copyright 2024 Vulcalien
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
#include "data-structs/array-list.h"

struct ArrayList {
    u32 size;
    u32 growth_step;
    void **values;

    u32 count;
};

struct ArrayList *arraylist_create(u32 initial_size,
                                   u32 growth_step) {
    struct ArrayList *result = malloc(sizeof(struct ArrayList));

    *result = (struct ArrayList) {
        .size = initial_size,
        .growth_step = growth_step,
        .values = calloc(initial_size, sizeof(void *)),

        .count = 0,
    };

    return result;
}

void arraylist_destroy(struct ArrayList *list,
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

void arraylist_add(struct ArrayList *list, void *value) {
    // if the array is full, increase its size
    if(list->count == list->size) {
        list->size += list->growth_step;
        list->values = realloc(
            list->values, list->size * sizeof(void *)
        );

        // fill the new cells with NULL
        for(u32 i = list->count; i < list->size; i++)
            list->values[i] = NULL;
    }

    list->values[list->count] = value;
    list->count++;
}

void *arraylist_get(struct ArrayList *list, u32 index) {
    if(index >= list->count)
        return NULL;

    return list->values[index];
}

u32 arraylist_count(struct ArrayList *list) {
    return list->count;
}

// The elements inside the list are not leaked: they will be destroyed
// when the list itself is destroyed.
void arraylist_clear(struct ArrayList *list) {
    list->count = 0;
}
