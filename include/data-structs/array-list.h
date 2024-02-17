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
#ifndef VULC_LUAG_ARRAY_LIST
#define VULC_LUAG_ARRAY_LIST

#include "luag-console.h"

struct ArrayList;

extern struct ArrayList *arraylist_create(u32 initial_size,
                                          u32 growth_step);

// if destroy_value_fn is NULL, the objects are not destroyed
extern void arraylist_destroy(struct ArrayList *list,
                              void (*destroy_value_fn)(void *));

// Inserts the element at the end of the list.
extern void arraylist_add(struct ArrayList *list, void *value);

extern void *arraylist_get(struct ArrayList *list, u32 index);

extern u32 arraylist_count(struct ArrayList *list);
extern void arraylist_clear(struct ArrayList *list);

#endif // VULC_LUAG_ARRAY_LIST
