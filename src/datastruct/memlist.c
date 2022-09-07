#include "memlist.h"

#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>

static const size_t MEMLIST_INITIAL_SIZE = 32;
static const memlist MEMLIST_INVALID = (memlist){0};
static const memlist_handle MEMLIST_HANDLE_INVALID = (memlist_handle){0};

memlist memlist_create(void) {
  return (memlist){.ptrlist = malloc(sizeof(void *) * MEMLIST_INITIAL_SIZE),
                   .size = MEMLIST_INITIAL_SIZE};
}

bool memlist_is_valid(memlist *mlp) {
  return mlp != (void *)0 && mlp->ptrlist != (void *)0;
}

void *memlist_P(memlist_handle mlh) {
  if (!memlist_is_valid(mlh.mlp) || mlh.id >= mlh.mlp->next_index)
    return (void *)0;
  return mlh.mlp->ptrlist[mlh.id];
}

bool memlist_handle_is_valid(memlist_handle mlh) {
  return mlh.mlp != (void *)0 && memlist_P(mlh) != (void *)0;
}

memlist_handle memlist_alloc(memlist *mlp, size_t size) {
  if (!memlist_is_valid(mlp)) return MEMLIST_HANDLE_INVALID;

  // Disable SIGINT handler so it can't interfere.
  void (*sigint_handler)(int) = signal(SIGINT, SIG_IGN);

  memlist_handle mlh = MEMLIST_HANDLE_INVALID;
  void *mem = malloc(size);
  if (mem) {
    if (mlp->next_index >= mlp->size) {
      size_t newsize = mlp->size * 2;
      mlp->ptrlist = realloc(mlp->ptrlist, newsize);
      mlp->size = newsize;
    }
    if (memlist_is_valid(mlp)) {
      mlp->ptrlist[mlp->next_index] = mem;
      mlh = (memlist_handle){.mlp = mlp, .id = mlp->next_index};
      mlp->next_index++;
    }
  }

  // Reenable SIGINT handler.
  signal(SIGINT, sigint_handler);

  return mlh;
}

memlist_handle memlist_realloc(memlist_handle mlh, size_t size) {
  if (!memlist_handle_is_valid(mlh)) return mlh;

  // Disable SIGINT so it can't interfere.
  void (*sigint_handler)(int) = signal(SIGINT, SIG_IGN);

  mlh.mlp->ptrlist[mlh.id] = realloc(mlh.mlp->ptrlist[mlh.id], size);

  // Reenable SIGINT handler.
  signal(SIGINT, sigint_handler);

  return mlh;
}

void memlist_free_one(memlist_handle mlh) {
  if (memlist_handle_is_valid(mlh)) {
    free(mlh.mlp->ptrlist[mlh.id]);
    mlh.mlp->ptrlist[mlh.id] = (void *)0;
  }
}

void memlist_destroy(memlist *mlp) {
  if (!memlist_is_valid(mlp)) return;
  for (int i = 0; i < mlp->next_index; i++) {
    if (mlp->ptrlist[i]) {
      free(mlp->ptrlist[i]);
      mlp->ptrlist[i] = (void *)0;
    }
  }
  free(mlp->ptrlist);
  mlp->ptrlist = (void *)0;
}
