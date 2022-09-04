#ifndef DATASTRUCT_MEMLIST_H
#define DATASTRUCT_MEMLIST_H

#include <stdbool.h>
#include <stdlib.h>

typedef struct memlist {
  void **ptrlist;
  unsigned int next_index;
  size_t size;
} memlist;

typedef struct memlist_handle {
  memlist *mlp;
  unsigned int id;
} memlist_handle;

memlist memlist_create(void);
bool memlist_is_valid(memlist *mlp);
void *memlist_P(memlist_handle mlh);
bool memlist_handle_is_valid(memlist_handle mlh);
memlist_handle memlist_alloc(memlist *mlp, size_t size);
memlist_handle memlist_realloc(memlist_handle mlh, size_t size);
void memlist_free_one(memlist_handle mlh);
void memlist_free_all(memlist *mlp);
void memlist_destroy(memlist *mlp);

#endif
