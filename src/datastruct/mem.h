/**
 * @file mem.h
 * @brief Memory allocation routines.
 *
 * This library provides dynamic memory allocation routines that can use
 * multiple types of allocators. It provides two allocator types: a plain
 * allocator that wraps `malloc` et al., and a memory table data structure.
 * `datastruct` data structures accept an allocator in the constructor
 * argument, and use the given allocator for all internal memory management.
 *
 * A memory table is a memory allocator that remembers all allocations.
 * When a memory table is destroyed, all of its allocations are freed. This
 * makes it suitable for error recovery.
 *
 * Memory table operations guard against being interrupted by SIGINT, making it
 * suitable for interrupt signal handling. Typically, you create a memory table,
 * then set a SIGINT handler to `longjmp` to code that destroys the memory table
 * and aborts the operation. The operation should destroy the memory table on
 * clean exit as well.
 *
 * Allocation returns a handle that can be used to access the address of the
 * memory, reallocate the memory, and free the memory. The handle is small and
 * can be passed by value.
 *
 * A memory table does not know about destructors, and as such is only suitable
 * for Plain Old Data objects. A memory table can be the allocator for another
 * memory table. Naturally, the topmost memory table must use a plain allocator.
 *
 *   memtbl_handle mth = memtbl_create(mem_allocator_plain());
 *   if (!memtbl_is_valid(mth)) goto end;
 *   mem_allocator ma = mem_allocator_memtbl(mth);
 *
 *   mem_handle buf_handle = mem_alloc(ma, 128);
 *   if (!mem_p(buf_handle)) goto end;
 *   map_handle config_map = map_create(ma);
 *   if (!map_is_valid(config_map)) goto end;
 *   // ...
 *
 *   end:
 *   memtbl_destroy(mth);
 *
 * If you need to pass a pointer of memory that was not allocated by
 * `mem_alloc`, use `mem_handle_from_ptr`.
 */

#ifndef DATASTRUCT_MEM_H
#define DATASTRUCT_MEM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "map.h"

// Internal memtbl ID. Must be incrementable, and a key type of map.
typedef uint32_t memtbl_id;

// A memory table. Create with `memtbl_create`, destroy with `memtbl_destroy`.
typedef struct memtbl {
  memtbl_id next_id;
  map_handle mem_map_handle;
} memtbl;

// Handle for a memtbl.
typedef mem_handle memtbl_handle;

// Type tags for mem_allocator.
enum mem_allocator_type {
  MEM_ALLOCATOR_TYPE_INVALID,
  MEM_ALLOCATOR_TYPE_NOT_ALLOCATED,
  MEM_ALLOCATOR_TYPE_PLAIN,
  MEM_ALLOCATOR_TYPE_MEMTBL
};

// Allocator for `mem_alloc` and datastruct constructors.
typedef struct mem_allocator {
  enum mem_allocator_type allocator_type;
  union {
    struct {
      memtbl_handle handle;
    } memtbl_info;
  } info;
} mem_allocator;

// Simple allocators with no attached data.
extern const mem_allocator MEM_NOT_ALLOCATED;
extern const mem_allocator MEM_ALLOCATOR_PLAIN;

// Handle for an allocation.
typedef struct mem_handle {
  mem_allocator allocator;
  union {
    struct {
      memtbl_id id;
    } memtbl_info;
    struct {
      void *ptr;
    } plain_info;
  } info;
  size_t size;
  bool allocated;
} mem_handle;

inline mem_handle mem_handle_from_ptr(void *ptr, size_t size) {
  return (mem_handle){
      .allocator = MEM_NOT_ALLOCATED, .info.plain_info.ptr = ptr, .size = size};
}

/**
 * @returns a `mem_allocator` that uses a given memtbl.
 */
inline mem_allocator mem_allocator_memtbl(memtbl_handle mth) {
  return (mem_allocator){.allocator_type = MEM_ALLOCATOR_TYPE_MEMTBL,
                         .info.memtbl_info.handle = mth};
}

/**
 * @brief Allocates memory.
 *
 * @param ma The memory allocator, from either `mem_allocator_plain` or
 *   `mem_allocator_memtbl`
 * @param size The amount of memory to allocate
 * @return A memory handle, possibly invalid. Test `mem_p(handle)` for null
 * before using.
 */
mem_handle mem_alloc(mem_allocator ma, size_t size);

/**
 * @brief Allocates memory, cleared with zeroes.
 *
 * @param ma The memory allocator, from either `mem_allocator_plain` or
 *   `mem_allocator_memtbl`
 * @param size The amount of memory to allocate
 * @return A memory handle, possibly invalid. Test `mem_p(handle)` for null
 * before using.
 */
mem_handle mem_alloc_clear(mem_allocator ma, size_t size);

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
 * After using, discard the original memory handle. Technically it is safe
 * to free an already-freed memory handle when using a memtbl allocator, but
 * it is not safe when using the plain allocator.
 *
 * @param handle The memory handle to free
 */
void mem_free(mem_handle handle);

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
 * @param handle The memory handle
 * @return true if the memory handle is valid
 */
inline bool mem_is_valid(mem_handle handle) {
  return (!mem_p(handle));
}

/**
 * @brief Duplicates a memory region.
 *
 * @param handle The memory to duplicate
 * @return mem_handle The duplicated memory, or an invalid handle on failure
 */
mem_handle mem_duplicate(mem_handle handle);

/**
 * @brief Creates a memory table.
 *
 * Use `memtbl_is_valid` to confirm that the table is valid before using.
 * Functions will fail gracefully if called with an invalid table.
 *
 * @param ma A memory allocator to use for this memtbl
 * @return memtbl The memory table, possibly invalid
 */
memtbl_handle memtbl_create(mem_allocator ma);

/**
 * @param mth Handle of the memory table.
 * @return true if the memory table is valid.
 */
bool memtbl_is_valid(memtbl_handle mth);

/**
 * @brief Destroys a memory table and deallocates all un-freed entries.
 *
 * The memtbl is updated so that it is no longer valid.
 *
 * @param mth Handle of the memory table to destroy.
 */
void memtbl_destroy(memtbl_handle mth);

#endif