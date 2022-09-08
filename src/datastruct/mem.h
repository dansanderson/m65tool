/**
 * @file mem.h
 * @brief Memory allocation routines.
 *
 * These routines wraps memory allocation routines (malloc, calloc, realloc,
 * free) with a memory allocator abstraction, and wraps pointers with a memory
 * handle abstraction. An allocator can have a pointer payload, and can attach a
 * pointer payload to each memory handle.
 *
 * This is primarily intended to support both plain allocation and memory table
 * allocation (see memtbl.h) throughout the datastruct library. The user
 * provides an allocator to a constructor, and the object uses that allocator
 * throughout its lifetime.
 *
 * Memory operations guard against being interrupted by SIGINT to keep the
 * internal state of an allocator consistent. This allows a memtbl allocator
 * to be used as part of a SIGINT handler, to abort an operation cleanly.
 *
 * Allocation returns a handle that can be used to access the address of the
 * memory, reallocate the memory, and free the memory. The handle is small and
 * can be passed by value, similar to a pointer. The handle stores the size of
 * the memory region, and information about the allocator.
 *
 * You can wrap unowned pointers with a memory handle for use with datastruct
 * operations. `mem_realloc` and `mem_free` do nothing when given such a handle.
 * See `mem_handle_from_ptr`.
 */

#ifndef DATASTRUCT_MEM_H
#define DATASTRUCT_MEM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "map.h"

// Type tags for mem_allocator.
enum mem_allocator_type {
  MEM_ALLOCATOR_TYPE_INVALID = 0,
  MEM_ALLOCATOR_TYPE_NOT_ALLOCATED,
  MEM_ALLOCATOR_TYPE_PLAIN,
  MEM_ALLOCATOR_TYPE_MEMTBL  // memtbl.h
};

typedef struct mem_allocator_spec {
  enum mem_allocator_type allocator_type;
  mem_handle (*alloc_func)(mem_allocator allocator, size_t size);
  mem_handle (*realloc_func)(mem_handle handle, size_t size);
  mem_handle (*free_func)(mem_handle handle);
  void *(*p_func)(mem_handle handle);
} mem_allocator_spec;

// Allocator for `mem_alloc` and datastruct constructors.
typedef struct mem_allocator {
  mem_allocator_spec *allocator_spec;
  void *allocator_data;
} mem_allocator;

// Simple allocators with no attached data.
extern const mem_allocator MEM_ALLOCATOR_NOT_ALLOCATED;
extern const mem_allocator MEM_ALLOCATOR_PLAIN;

// Handle for an allocation.
typedef struct mem_handle {
  void *data;
  size_t size;

  mem_allocator allocator;
} mem_handle;

/**
 * @return mem_handle for an unowned region of memory
 */
inline mem_handle mem_handle_from_ptr(void *ptr, size_t size);

/**
 * @brief Allocates memory.
 *
 * @param ma The memory allocator, from either `mem_allocator_plain` or
 *   `mem_allocator_memtbl`
 * @param size The amount of memory to allocate
 * @return A memory handle, possibly invalid. Test `mem_p(handle)` for null
 * before using.
 */
mem_handle mem_alloc(mem_allocator allocator, size_t size);

/**
 * @brief Allocates memory, cleared with zeroes.
 *
 * @param ma The memory allocator, from either `mem_allocator_plain` or
 *   `mem_allocator_memtbl`
 * @param size The amount of memory to allocate
 * @return A memory handle, possibly invalid. Test `mem_p(handle)` for null
 * before using.
 */
mem_handle mem_alloc_clear(mem_allocator allocator, size_t size);

/**
 * @brief Reallocates memory.
 *
 * After using, replace the original memory handle with the new one returned
 * by this function. Technically you can reuse the original memory handle
 * value when using a memtbl allocator, but you must use the new handle with
 * the plain allocator. Doing so in both cases makes your code compatible
 * with both allocators.
 *
 * In all cases, call `mem_p(handle)` again after the reallocation to access the
 * address, which may have changed (or become `(void *)0` on failure) after
 * the reallocation.
 *
 * @param handle The original memory handle
 * @param size The new desired size of the memory allocation
 * @return mem_handle A new memory handle
 */
mem_handle mem_realloc(mem_handle handle, size_t size);

/**
 * @brief Frees an allocation of memory.
 *
 * After using, discard the original memory handle. If necessary, store
 * the value returned by `mem_free` (an invalid handle) to avoid reusing a freed
 * handle.
 *
 * @param handle The memory handle to free
 * @return mem_handle An invalid mem_handle. Replace the freed handle with this
 *   storage, or ignore it if not needed.
 */
mem_handle mem_free(mem_handle handle);

/**
 * @brief Accesses the address of memory for a handle.
 *
 * This returns `(void *)0` if the handle is invalid, such as after a failed
 * allocation.
 *
 * @param handle The memory handle
 * @return void* The address of the allocated memory
 */
void *mem_p(mem_handle handle);

/**
 * @return size_t The size of the memory region
 */
inline size_t mem_size(mem_handle handle);

/**
 * @param handle The memory handle
 * @return true if the memory handle is valid
 */
inline bool mem_is_valid(mem_handle handle);

/**
 * @brief Duplicates a memory region.
 *
 * This uses the same allocator as the original handle. To use a different
 * allocator for the duplicate, see `mem_duplicate_with_allocator`.
 *
 * @param handle The memory to duplicate
 * @return mem_handle The duplicated memory, or an invalid handle on failure
 */
inline mem_handle mem_duplicate(mem_handle handle);

/**
 * @brief Duplicates a memory region using a given allocator.
 *
 * @param allocator The allocator to use for the duplicate
 * @param handle The memory to duplicate
 * @return mem_handle The duplicated memory, or an invalid handle on failure
 */
mem_handle mem_duplicate_with_allocator(mem_allocator allocator,
                                        mem_handle handle);

#endif