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
#ifndef VULC_LUAG_CIRCULAR_LIST
#define VULC_LUAG_CIRCULAR_LIST

#include "luag-console.h"

struct CircularList;

extern struct CircularList *circularlist_create(u32 size);

// if destroy_value_fn is NULL, the objects are not destroyed
extern void circularlist_destroy(struct CircularList *list,
                                 void (*destroy_value_fn)(void *));

// Inserts the element at index 0.
// If destroy_value_fn is NULL, the overwritten object is not destroyed
extern void circularlist_add(struct CircularList *list, void *value,
                             void (*destroy_value_fn)(void *));

extern void *circularlist_get(struct CircularList *list, u32 index);

extern u32 circularlist_count(struct CircularList *list);
extern void circularlist_clear(struct CircularList *list);

#endif // VULC_LUAG_CIRCULAR_LIST
