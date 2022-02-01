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
#include "hashtable.h"

#include <string.h>

struct hashtable_Entry {
    char *key;
    void *value;

    struct hashtable_Entry *next;
};

struct Hashtable {
    struct hashtable_Entry *slots;
    u32 size;
};

static u32 hash_string(const char *string, u32 table_size) {
    u32 hash = 0;

    for(u32 i = 0; string[i] != '\0'; i++) {
        hash = hash * 31 + string[i];
    }
    return hash % table_size;
}

static struct hashtable_Entry *get_entry(struct Hashtable *table,
                                         const char *key) {
    u32 hash = hash_string(key, table->size);
    return &table->slots[hash];
}

struct Hashtable *hashtable_create(u32 size) {
    struct Hashtable *table = malloc(sizeof(struct Hashtable));

    *table = (struct Hashtable) {
        .slots = calloc(size, sizeof(struct hashtable_Entry)),
        .size = size
    };
    return table;
}

static void destroy_entry(struct hashtable_Entry *entry,
                          void (*destroy_value_fn)(void *)) {
    if(entry->key)
        free(entry->key);
    if(entry->value && destroy_value_fn)
        destroy_value_fn(entry->value);

    if(entry->next) {
        destroy_entry(entry->next, destroy_value_fn);
        free(entry->next);
    }
}

void hashtable_destroy(struct Hashtable *table,
                       void (*destroy_value_fn)(void *)) {
    for(u32 i = 0; i < table->size; i++)
        destroy_entry(&table->slots[i], destroy_value_fn);
    free(table->slots);
    free(table);
}

void hashtable_set(struct Hashtable *table,
                   const char *key, void *value) {
    struct hashtable_Entry *entry = get_entry(table, key);

    while(entry->next)
        entry = entry->next;

    if(entry->key) {
        entry->next = calloc(1, sizeof(struct hashtable_Entry));
        entry = entry->next;
    }

    entry->key = malloc((strlen(key) + 1) * sizeof(char));
    strcpy(entry->key, key);

    entry->value = value;
}

int hashtable_get(struct Hashtable *table,
                  const char *key, void **value) {
    struct hashtable_Entry *entry = get_entry(table, key);

    while(true) {
        if(entry->key && !strcmp(key, entry->key)) {
            *value = entry->value;
            return 0;
        }

        if(!entry->next) {
            *value = NULL;
            return -1;
        }

        entry = entry->next;
    }
}
