/**
 * @file map.h
 * @brief A map data structure.
 *
 * This map data structure can use `str` keys or `void *` keys.
 *
 * The value of an entry is a mem_handle. A map never owns the memory of the
 * value: when a key is deleted or the map is destroyed, the map does not free
 * entry memory. If you don't want to keep track of entry memory, use a memtbl
 * and duplicate mem_handles as needed before adding them to the map.
 */

#ifndef DATASTRUCT_MAP_H
#define DATASTRUCT_MAP_H

#include <stdbool.h>
#include <stdint.h>

#include "mem.h"
#include "str.h"

// Handle for a map, returned by `map_create`
typedef mem_handle map_handle;

// Internal type for a map entry
typedef struct map_entry map_entry;

// Internal type for a map data structure
typedef struct map {
  mem_handle entries_mh;
  unsigned int entry_count;
  unsigned int table_size;
} map;

/**
 * @brief Creates a map.
 *
 * Use `map_is_valid` to validate the map before using.
 *
 * @param ma The memory allocator to use
 * @return map_handle A handle for the map
 */
map_handle map_create(mem_allocator ma);

/**
 * @param mh The map handle
 * @return true if the map is valid
 */
bool map_is_valid(map_handle mh);

/**
 * @brief Destroys a map.
 *
 * @param mh The handle of the map to destroy
 */
void map_destroy(map_handle mh);

// clang-format off
/**
 * @brief Sets a key-value pair in the map.
 *
 * If this returns false, the entry was not set successfully, possibly due to
 * an out-of-memory error while resizing the internal table.
 *
 * @param mh The map_handle
 * @param key The key, either a str or a void*
 * @param value The mem_handle value
 * @return true on success
 */
#define map_set(mh, key, value) \
  _Generic((key), \
    str: map_set_str, \
    void *: map_set_ptr \
  )((mh), (key), (value))
// clang-format on
bool map_set_str(map_handle mh, str key, mem_handle value);
bool map_set_ptr(map_handle mh, void *key, mem_handle value);

// clang-format off
/**
 * @brief Gets a value for a key in a map.
 *
 * @param mh The map_handle
 * @param key The key, either a str or a void*
 * @return mem_handle The value stored, or an invalid mem_handle if not found
 */
#define map_get(mh, key) \
  _Generic((key), \
    str: map_get_str, \
    void *: map_get_ptr \
  )((mh), (key))
// clang-format on
mem_handle map_get_str(map_handle mh, str key);
mem_handle map_get_ptr(map_handle mh, void *key);

// clang-format off
/**
 * @brief Deletes an entry from a map.
 *
 * If this returns false, there was an out-of-memory error while attempting to
 * shrink the internal table. The value is still deleted from the map.
 *
 * @param mh The map_handle
 * @param key The key, either a str or a void*
 * @return false if there was a memory error while shrinking the internal table
 */
#define map_delete(mh, key) \
  _Generic((key), \
    str: map_delete_str, \
    void *: map_delete_ptr \
  )((mh), (key))
// clang-format on
bool map_delete_str(map_handle mh, str key);
bool map_delete_ptr(map_handle mh, void *key);

// Map iterator
typedef struct map_iter {
  map_handle mh;
  unsigned int pos;
  mem_handle value_handle;
} map_iter;

/**
 * @brief Gets the first value iterator in a map iteration.
 *
 * Map values have no guaranteed order. If anything adds or deletes an element,
 * using an existing `map_iter` is undefined.
 *
 * @param mh
 * @return map_iter
 */
map_iter map_first_value_iter(map_handle mh);

/**
 * @brief Gets the next value iterator in a map iteration.
 *
 * See `map_first_value_iter`.
 *
 * @param it
 * @return map_iter
 */
map_iter map_next_value_iter(map_iter it);

/**
 * @param it
 * @return true if the iterator does not point to a value and has no values
 *   after it.
 */
bool map_iter_done(map_iter it);

/**
 * @brief Gets the map value pointed at by an iterator.
 *
 * @param it
 * @return mem_handle
 */
mem_handle map_iter_value(map_iter it);

#endif
