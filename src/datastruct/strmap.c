#include "strmap.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "memlist.h"

struct strmap_entry {
  str key;
  void *value;
  bool allocated;
  memlist_handle value_mlh;
};

static const unsigned int INITIAL_TABLE_SIZE = 32;
static const strmap INVALID_STRMAP = (strmap){0};

strmap strmap_create(void) {
  strmap_entry *entries = calloc(INITIAL_TABLE_SIZE, sizeof(strmap_entry));
  if (!entries) return INVALID_STRMAP;
  return (strmap){.entries = entries,
                  .entry_count = 0,
                  .table_size = INITIAL_TABLE_SIZE,
                  .mlp = (void *)0,
                  .entries_mlh = (memlist_handle){0}};
}

strmap strmap_create_to_memlist(memlist *mlp) {
  if (!memlist_is_valid(mlp)) return INVALID_STRMAP;
  memlist_handle mlh =
      memlist_alloc(mlp, sizeof(strmap_entry) * INITIAL_TABLE_SIZE);
  if (!memlist_handle_is_valid(mlh)) return INVALID_STRMAP;
  memset(memlist_P(mlh), 0, sizeof(strmap_entry) * INITIAL_TABLE_SIZE);
  return (strmap){.entries = memlist_P(mlh),
                  .entry_count = 0,
                  .table_size = INITIAL_TABLE_SIZE,
                  .mlp = mlp,
                  .entries_mlh = mlh};
}

bool strmap_is_valid(strmap map) {
  return map.entries != (void *)0;
}

void strmap_destroy(strmap *mapptr) {
  if (mapptr->mlp) {
    for (int i = 0; i < mapptr->table_size; i++) {
      if (mapptr->entries[i].value && mapptr->entries[i].allocated) {
        memlist_free_one(mapptr->entries[i].value_mlh);
      }
    }
    memlist_free_one(mapptr->entries_mlh);
  } else {
    for (int i = 0; i < mapptr->table_size; i++) {
      if (mapptr->entries[i].value && mapptr->entries[i].allocated) {
        free(mapptr->entries[i].value);
      }
    }
    free(mapptr->entries);
  }
  mapptr->entries = (void *)0;
}

// Fowler/Noll/Vo 32-bit FNV-1a hash
// Based on http://isthe.com/chongo/tech/comp/fnv/
static uint32_t hash_key(str key) {
  uint32_t hash = 0x811c9dc5;
  for (int i = 0; i < key.length; i++) {
    char c = key.value[i];
    hash ^= (uint32_t)c;
    // GCC optimized equivalent to hash *= 0x01000193 :
    hash +=
        (hash << 1) + (hash << 4) + (hash << 7) + (hash << 8) + (hash << 24);
  }
  return hash;
}

static void rehash_entries_table(strmap_entry *oldtbl, unsigned int oldtbl_size,
                                 strmap_entry *newtbl,
                                 unsigned int newtbl_size) {
  for (int old_i = 0; old_i < oldtbl_size; old_i++) {
    if (oldtbl[old_i].value) {
      unsigned int table_index = hash_key(oldtbl[old_i].key) % newtbl_size;
      unsigned int new_i = table_index;
      do {
        if (!newtbl[new_i].value) {
          newtbl[new_i] = oldtbl[old_i];
          break;
        }
        new_i = (new_i + 1) % newtbl_size;
      } while (new_i != table_index);
      // Assert that we found an empty slot.
      assert(newtbl[new_i].value == oldtbl[old_i].value);
    }
  }
}

static bool grow_table(strmap *mapptr) {
  unsigned int newsize = mapptr->table_size * 2;

  if (mapptr->mlp != (void *)0) {
    memlist_handle newmlh =
        memlist_alloc(mapptr->mlp, sizeof(strmap_entry) * newsize);
    if (!memlist_handle_is_valid(newmlh)) return false;
    strmap_entry *newentries = memlist_P(newmlh);
    rehash_entries_table(mapptr->entries, mapptr->table_size, newentries,
                         newsize);
    memlist_free_one(mapptr->entries_mlh);
    mapptr->entries = newentries;
    mapptr->entries_mlh = newmlh;

  } else {
    strmap_entry *newentries = malloc(sizeof(strmap_entry) * newsize);
    if (!newentries) return false;
    rehash_entries_table(mapptr->entries, mapptr->table_size, newentries,
                         newsize);
    free(mapptr->entries);
    mapptr->entries = newentries;
  }

  mapptr->table_size = newsize;
  return true;
}

static bool do_strmap_set(strmap *mapptr, str key, void *value, bool allocated,
                          memlist_handle value_mlh) {
  unsigned int table_index = hash_key(key) % mapptr->table_size;
  unsigned int i = table_index;
  do {
    if (!mapptr->entries[i].value) {
      mapptr->entries[i] = (strmap_entry){.key = key,
                                          .value = value,
                                          .allocated = allocated,
                                          .value_mlh = value_mlh};
      mapptr->entry_count++;
      if (mapptr->entry_count > mapptr->table_size / 2) {
        if (!grow_table(mapptr)) return false;
      }
      return true;

    } else if (str_compare(key, mapptr->entries[i].key) == 0) {
      if (mapptr->entries[i].allocated) {
        if (mapptr->mlp) {
          memlist_free_one(mapptr->entries[i].value_mlh);
        } else {
          free(mapptr->entries[i].value);
        }
      }
      mapptr->entries[i] = (strmap_entry){.key = key,
                                          .value = value,
                                          .allocated = allocated,
                                          .value_mlh = value_mlh};
      return true;
    }

    i = (i + 1) % mapptr->table_size;
  } while (i != table_index);

  // Table completely full. This shouldn't happen.
  assert(false);
  return false;
}

bool strmap_set(strmap *mapptr, str key, void *value) {
  return do_strmap_set(mapptr, key, value, false, (memlist_handle){0});
}

bool strmap_set_copy(strmap *mapptr, str key, void *value, size_t size) {
  void *copy;
  memlist_handle copy_mlh = (memlist_handle){0};
  if (mapptr->mlp != (void *)0) {
    copy_mlh = memlist_alloc(mapptr->mlp, size);
    if (!memlist_handle_is_valid(copy_mlh)) return false;
    copy = memlist_P(copy_mlh);
  } else {
    copy = malloc(size);
    if (!copy) return false;
  }
  memcpy(copy, value, size);
  return do_strmap_set(mapptr, key, copy, true, copy_mlh);
}

void *strmap_get(strmap *mapptr, str key) {
  unsigned int table_index = hash_key(key) % mapptr->table_size;
  unsigned int i = table_index;
  do {
    if (!mapptr->entries[i].value) {
      return (void *)0;
    } else if (str_compare(key, mapptr->entries[i].key) == 0) {
      return mapptr->entries[i].value;
    }
    i = (i + 1) % mapptr->table_size;
  } while (i != table_index);

  // Table completely full. This shouldn't happen.
  assert(false);
  return (void *)0;
}
