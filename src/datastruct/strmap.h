#ifndef DATASTRUCT_STRMAP_H
#define DATASTRUCT_STRMAP_H

#include <stdbool.h>

#include "datastruct/memlist.h"
#include "datastruct/str.h"

typedef struct strmap_entry strmap_entry;

typedef struct strmap {
  strmap_entry *entries;
  unsigned int entry_count;
  unsigned int table_size;
  memlist *mlp;
  memlist_handle entries_mlh;
} strmap;

strmap strmap_create(void);
strmap strmap_create_to_memlist(memlist *mlp);
bool strmap_is_valid(strmap map);
void strmap_destroy(strmap *mapptr);
bool strmap_set(strmap *mapptr, str key, void *value);
bool strmap_set_copy(strmap *mapptr, str key, void *value, size_t size);
void *strmap_get(strmap *mapptr, str key);

#endif
