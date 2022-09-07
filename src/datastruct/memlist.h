#ifndef DATASTRUCT_MEMLIST_H
#define DATASTRUCT_MEMLIST_H

#include <stdbool.h>
#include <stdlib.h>

/**
 * @brief A memory allocator that can free all allocations at once safely.
 */
typedef struct memlist {
  // Array of pointers to allocation memory regions
  void **ptrlist;

  // The next unused index in ptrlist
  unsigned int next_index;

  // The current size of ptrlist
  size_t size;
} memlist;

/**
 * @brief A handle representing a memlist allocation.
 */
typedef struct memlist_handle {
  // Reference to the memlist that created this
  memlist *mlp;

  // ID of the memory allocation that this handle represents
  unsigned int id;
} memlist_handle;

/**
 * @brief Creates a memlist.
 *
 * Use `memlist_is_valid` to confirm that the memlist was allocated correctly.
 *
 * Use `memlist_destroy` to deallocate the memlist and all of its allocations.
 * Use `memlist_free_all` to deallocate only the allocations and reset the
 * memlist.
 *
 * @return memlist The memlist value
 */
memlist memlist_create(void);

/**
 * @return true if the memlist is valid.
 */
bool memlist_is_valid(memlist *mlp);

/**
 * @brief The raw pointer of a memlist entry for a given handle.
 *
 * @param mlh The memlist handle returned by `memlist_alloc`
 * @return void* The pointer to the allocated memory
 */
void *memlist_P(memlist_handle mlh);

/**
 * @return true if the memlist handle is valid and refers to allocated memory
 */
bool memlist_handle_is_valid(memlist_handle mlh);

/**
 * @brief Allocates memory and records it in the memlist.
 *
 * @param mlp Ptr to the memlist
 * @param size The size of the memory to allocate
 * @return memlist_handle A handle representing the allocated memory, or an
 *   invalid handle if allocation failed (see `memset_handle_is_valid`).
 */
memlist_handle memlist_alloc(memlist *mlp, size_t size);

/**
 * @brief Reallocates memory for a memlist handle.
 *
 * The reallocation may or may not change the memory pointer for the handle. Be
 * sure to use `memlist_P` with the handle again after reallocation to get the
 * updated pointer.
 *
 * Use `memlist_handle_is_valid` to confirm that reallocation was successful.
 *
 * This returns a memlist_handle identical to the argument, for convenience.
 * You can ignore the return value and just test the original handle.
 *
 * @param mlh The memlist handle
 * @param size The new desired size
 * @return memlist_handle The handle (same as mlh).
 */
memlist_handle memlist_realloc(memlist_handle mlh, size_t size);

/**
 * @brief Frees a single entry on the memlist from its handle.
 *
 * It is safe to free an already freed entry. This does nothing. List entries
 * are not reused, so a freed entry will remain freed for the lifetime of the
 * memlist.
 *
 * Freeing an entry invalidates all handle values that point to the entry.
 * `memlist_handle_is_valid` returns false after the entry has been freed.
 *
 * @param mlh The handle of the entry to free.
 */
void memlist_free_one(memlist_handle mlh);

/**
 * @brief Frees all entries in the memlist, and frees and invalidates the
 * memlist.
 *
 * @param mlp Ptr to the memlist to destroy
 */
void memlist_destroy(memlist *mlp);

#endif
