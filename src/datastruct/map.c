#include "map.h"

#include <assert.h>
#include <stdint.h>

#include "str.h"

static const unsigned int INITIAL_TABLE_SIZE = 32;

enum map_key_type { MAP_KEY_STR, MAP_KEY_PTR };

struct map_entry {
  uint32_t key_hash;
  mem_handle value_handle;
};

map_handle map_create(mem_allocator ma) {
  map_handle mh = mem_alloc(ma, sizeof(map));
  if (!mem_is_valid(mh)) return (map_handle){0};
  map *mp = mem_p(mh);
  mp->entry_count = 0;
  mp->table_size = INITIAL_TABLE_SIZE;
  mp->entries_mh = mem_alloc_clear(ma, sizeof(map_entry) * INITIAL_TABLE_SIZE);
  if (!mem_is_valid(mp->entries_mh)) {
    mem_free(mh);
    return (map_handle){0};
  }
  return mh;
}

bool map_is_valid(map_handle mh) {
  return mem_is_valid(mh) && mem_is_valid(((map *)mem_p(mh))->entries_mh);
}

void map_destroy(map_handle mh) {
  if (!map_is_valid(mh)) return;
  map *mp = mem_p(mh);
  mem_free(mp->entries_mh);
  mem_free(mh);
}

/**
 * @brief Hash a str to a uint32.
 *
 * This uses Fowler/Noll/Vo 32-bit FNV-1a hash, based on:
 * http://isthe.com/chongo/tech/comp/fnv/
 *
 * The key type is included in the hashed value, i.e. a str and uint32 with the
 * same bytes will hash to different values.
 *
 * @param key
 * @return uint32_t
 */
static uint32_t hash_str(str key) {
  uint32_t hash = 0x811c9dc5;

  char c = MAP_KEY_STR;
  hash ^= (uint32_t)c;
  // GCC optimized equivalent to hash *= 0x01000193 :
  hash += (hash << 1) + (hash << 4) + (hash << 7) + (hash << 8) + (hash << 24);

  for (unsigned int i = 0; i < key.size; i++) {
    c = ((char *)mem_p(key))[i];
    hash ^= (uint32_t)c;
    // GCC optimized equivalent to hash *= 0x01000193 :
    hash +=
        (hash << 1) + (hash << 4) + (hash << 7) + (hash << 8) + (hash << 24);
  }
  return hash;
}

/**
 * @brief Hash a memory address to a uint32.
 *
 * This uses Fowler/Noll/Vo 32-bit FNV-1a hash, based on:
 * http://isthe.com/chongo/tech/comp/fnv/
 *
 * The key type is included in the hashed value, i.e. a str and uint32 with the
 * same bytes will hash to different values.
 *
 * @param key
 * @return uint32_t
 */
static uint32_t hash_ptr(void *key) {
  uint32_t hash = 0x811c9dc5;

  char c = MAP_KEY_PTR;
  hash ^= (uint32_t)c;
  // GCC optimized equivalent to hash *= 0x01000193 :
  hash += (hash << 1) + (hash << 4) + (hash << 7) + (hash << 8) + (hash << 24);

  uintptr_t keyint = (uintptr_t)key;

  for (unsigned int i = 0; i < sizeof(uintptr_t); i++) {
    c = keyint & 255;
    keyint = keyint >> 8;
    hash ^= (uint32_t)c;
    // GCC optimized equivalent to hash *= 0x01000193 :
    hash +=
        (hash << 1) + (hash << 4) + (hash << 7) + (hash << 8) + (hash << 24);
  }
  return hash;
}

/**
 * @brief Resizes the entries table.
 *
 * If is_grow is true, this doubles the size of the entries table. If is_grow
 * is false, this halves the size of the entries table. It's up to the caller
 * to only do this under appropriate conditions.
 *
 * @param mh The map handle
 * @param is_grow true if growing, otherwise shrinking
 * @return true on success
 */
static bool resize_entries_table(map_handle mh, bool is_grow) {
  map *mp = mem_p(mh);
  unsigned int new_table_size = mp->table_size * (is_grow ? 2 : 0.5);
  mem_handle new_entries_mh =
      mem_alloc_clear(mh.allocator, sizeof(map_entry) * new_table_size);
  if (!mem_p(new_entries_mh)) return false;

  map_entry *old_entries = mem_p(mp->entries_mh);
  map_entry *new_entries = mem_p(new_entries_mh);
  for (unsigned int i = 0; i < mp->table_size; i++) {
    if (!(old_entries + i)->key_hash) continue;
    unsigned int new_pos = (old_entries + i)->key_hash % new_table_size;
    *(new_entries + new_pos) = *(old_entries + i);
  }

  mem_free(mp->entries_mh);
  mp->table_size = new_table_size;
  mp->entries_mh = new_entries_mh;
  return true;
}

/**
 * @brief Locates the table position for a key.
 *
 * The position is either the position of an existing map_entry with the key,
 * or the next empty slot where a new entry with the key would go.
 *
 * @param mh
 * @param key_hash
 * @return unsigned int
 */
static unsigned int find_entry_pos(map_handle mh, uint32_t key_hash) {
  map *mp = mem_p(mh);
  map_entry *entries = mem_p(mp->entries_mh);
  unsigned int start_pos = key_hash % mp->table_size;
  unsigned int pos = start_pos;
  do {
    if (((entries + pos)->key_hash == key_hash) ||
        ((entries + pos)->key_hash == 0)) {
      return pos;
    }
    ++pos;
    if (pos >= mp->table_size) pos = 0;
  } while (pos != start_pos);
  assert(pos != start_pos);
  return -1;
}

static bool do_set(map_handle mh, uint32_t key_hash, mem_handle value) {
  map *mp = mem_p(mh);
  map_entry *entries = mem_p(mp->entries_mh);
  unsigned int pos = find_entry_pos(mh, key_hash);
  if ((entries + pos)->key_hash == 0) {
    ++mp->entry_count;
    if (mp->entry_count > (mp->table_size / 2)) {
      // A failed resize leaves the new entry unset.
      if (!resize_entries_table(mh, true)) return false;

      pos = find_entry_pos(mh, key_hash);
    }
  }
  (entries + pos)->key_hash = key_hash;
  (entries + pos)->value_handle = value;

  return true;
}

static mem_handle do_get(map_handle mh, uint32_t key_hash) {
  map *mp = mem_p(mh);
  map_entry *entries = mem_p(mp->entries_mh);
  unsigned int pos = find_entry_pos(mh, key_hash);
  if ((entries + pos)->key_hash == 0) return (mem_handle){0};
  return (entries + pos)->value_handle;
}

static bool do_delete(map_handle mh, uint32_t key_hash) {
  map *mp = mem_p(mh);
  map_entry *entries = mem_p(mp->entries_mh);
  unsigned int pos = find_entry_pos(mh, key_hash);
  if ((entries + pos)->key_hash == 0) return false;

  (entries + pos)->key_hash = 0;
  (entries + pos)->value_handle = (mem_handle){0};
  --mp->entry_count;

  // Shrink if entry count < 1/4th the table size. Note that this is not <=
  // to leave a one-element threshold, so an add followed by a delete does not
  // cause the table to grow then shrink immediately.
  if (mp->entry_count < (mp->table_size / 4) &&
      mp->table_size > INITIAL_TABLE_SIZE) {
    return resize_entries_table(mh, false);
  }
  return true;
}

bool map_set_str(map_handle mh, str key, mem_handle value) {
  if (!map_is_valid(mh)) return false;
  if (!mem_p(value)) return false;
  return do_set(mh, hash_str(key), value);
}

bool map_set_ptr(map_handle mh, void *key, mem_handle value) {
  if (!map_is_valid(mh)) return false;
  if (!mem_is_valid(value)) return false;
  return do_set(mh, hash_ptr(key), value);
}

mem_handle map_get_str(map_handle mh, str key) {
  if (!map_is_valid(mh)) return (mem_handle){0};
  return do_get(mh, hash_str(key));
}

mem_handle map_get_ptr(map_handle mh, void *key) {
  if (!map_is_valid(mh)) return (mem_handle){0};
  return do_get(mh, hash_ptr(key));
}

bool map_delete_str(map_handle mh, str key) {
  return do_delete(mh, hash_str(key));
}

bool map_delete_ptr(map_handle mh, void *key) {
  return do_delete(mh, hash_ptr(key));
}

map_iter map_first_value_iter(map_handle mh) {
  map *mp = mem_p(mh);
  if (!mp) return (map_iter){0};

  map_entry *first = mem_p(mp->entries_mh);
  map_iter it =
      (map_iter){.mh = mh, .pos = 0, .value_handle = first->value_handle};
  if (!mem_p(first->value_handle)) it = map_next_value_iter(it);
  return it;
}

map_iter map_next_value_iter(map_iter it) {
  map *mp = mem_p(it.mh);
  if (!mp) return (map_iter){0};

  map_entry *first = mem_p(mp->entries_mh);
  unsigned int pos = it.pos + 1;
  while (pos < mp->table_size &&
         mem_p((first + pos)->value_handle) == (void *)0)
    pos++;
  if (pos >= mp->table_size) return (map_iter){0};
  return (map_iter){
      .mh = it.mh, .pos = pos, .value_handle = (first + pos)->value_handle};
}

inline bool map_iter_done(map_iter it) {
  return mem_p(it.value_handle) == (void *)0;
}

inline mem_handle map_iter_value(map_iter it) {
  return it.value_handle;
}
