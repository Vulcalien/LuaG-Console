/* Copyright 2022 Vulcalien
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
#ifndef VULC_LUAG_HASHTABLE
#define VULC_LUAG_HASHTABLE

#include "luag-console.h"

struct Hashtable;

extern struct Hashtable *hashtable_create(u32 initial_size);

// if destroy_value_fn is NULL, the value object is not destroyed
extern void hashtable_destroy(struct Hashtable *table,
                              void (*destroy_value_fn)(void *));

extern void hashtable_set(struct Hashtable *table,
                          const char *key, void *value);

extern int hashtable_get(struct Hashtable *table,
                         const char *key, void **value);

#endif // VULC_LUAG_HASHTABLE
