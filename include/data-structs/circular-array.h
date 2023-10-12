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
#ifndef VULC_LUAG_CIRCULAR_ARRAY
#define VULC_LUAG_CIRCULAR_ARRAY

#include "luag-console.h"

struct CircularArray;

extern struct CircularArray *circulararray_create(u32 size);

// if destroy_value_fn is NULL, the objects are not destroyed
extern void circulararray_destroy(struct CircularArray *array,
                                  void (*destroy_value_fn)(void *));

// Inserts the element at index 0.
// If destroy_value_fn is NULL, the overwritten object is not destroyed
extern void circulararray_add(struct CircularArray *array, void *value,
                              void (*destroy_value_fn)(void *));

extern void *circulararray_get(struct CircularArray *array, u32 index);

extern u32 circulararray_count(struct CircularArray *array);
extern void circulararray_clear(struct CircularArray *array);

#endif // VULC_LUAG_CIRCULAR_ARRAY
