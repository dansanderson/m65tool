#include "mem.h"

/**
 * @brief A memory allocator that remembers allocations, and frees all when
 * destroyed.
 *
 *   memtbl_handle mth = memtbl_create(MEM_ALLOCATOR_PLAIN);
 *   if (!memtbl_is_valid(mth)) abort();
 *   mem_allocator allocator = mem_allocator_memtbl(mth);
 *
 *   mem_handle mem1 = mem_alloc(allocator, 32);
 *   mem_handle mem2 = mem_alloc(allocator, 32);
 *   mem_handle mem3 = mem_alloc(allocator, 32);
 *   mem_free(mem2);
 *
 *   memtbl_destroy(mth);  // frees mem1, mem3
 *
 * This is intended to be used in combination with a SIGINT handler and longjmp
 * to abort an operation cleanly. `mem_alloc` et al. are atomic with respect to
 * the SIGINT handler, so the memtbl remains in a consistent state.
 *
 * A memory table does not know about destructors, and as such is only suitable
 * for Plain Old Data objects. A memory table can be the allocator for another
 * memory table. Naturally, the topmost memory table must use a plain allocator.
 */

// A memory table. Create with `memtbl_create`, destroy with `memtbl_destroy`.
typedef struct memtbl {
  map_handle mem_map_handle;
} memtbl;

// Handle for a memtbl.
typedef mem_handle memtbl_handle;

/**
 * @brief Creates a memory table.
 *
 * Use `memtbl_is_valid` to confirm that the table is valid before using.
 * Functions will fail gracefully if called with an invalid table.
 *
 * @param allocator A memory allocator to use for this memtbl
 * @return memtbl The memory table, possibly invalid
 */
memtbl_handle memtbl_create(mem_allocator allocator);

/**
 * @param mthandle Handle of the memory table.
 * @return true if the memory table is valid.
 */
bool memtbl_is_valid(memtbl_handle mthandle);

/**
 * @brief Destroys a memory table and deallocates all un-freed entries.
 *
 * The memtbl is updated so that it is no longer valid.
 *
 * @param mth Handle of the memory table to destroy.
 */
void memtbl_destroy(memtbl_handle mthandle);

/**
 * @returns a `mem_allocator` that uses a given memtbl.
 */
inline mem_allocator mem_allocator_memtbl(memtbl_handle mthandle);
