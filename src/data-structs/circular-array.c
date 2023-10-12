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
#include "data-structs/circular-array.h"

// Implementation of a Circular Array of generic items. The array grows
// right-to-left and overwrites the elements as it grows.

struct CircularArray {
    u32 size;
    void **values;

    u32 count;
    i32 head;
};

struct CircularArray *circulararray_create(u32 size) {
    struct CircularArray *result = malloc(sizeof(struct CircularArray));

    *result = (struct CircularArray) {
        .size = size,
        .values = calloc(size, sizeof(void *)),

        .count = 0,
        .head = size - 1
    };

    return result;
}

void circulararray_destroy(struct CircularArray *array,
                           void (*destroy_value_fn)(void *)) {
    if(destroy_value_fn) {
        for(u32 i = 0; i < array->size; i++) {
            void *value = array->values[i];
            if(value)
                destroy_value_fn(value);
        }
    }

    free(array->values);
    free(array);
}

void circulararray_add(struct CircularArray *array, void *value,
                       void (*destroy_value_fn)(void *)) {
    if(array->count == array->size) {
        // delete the value present in that position, if present
        void *old = array->values[array->head];
        if(old && destroy_value_fn)
            destroy_value_fn(old);
    } else {
        array->count++;
    }

    array->values[array->head] = value;
    array->head--;
    if(array->head < 0)
        array->head = array->size - 1;
}

void *circulararray_get(struct CircularArray *array, u32 index) {
    if(index >= array->count)
        return NULL;

    return array->values[(array->head + 1 + index) % (array->size)];
}

u32 circulararray_count(struct CircularArray *array) {
    return array->count;
}

void circulararray_clear(struct CircularArray *array) {
    array->count = 0;
}
