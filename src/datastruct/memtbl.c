#include "memtbl.h"

#include "map.h"
#include "mem.h"

memtbl_handle memtbl_create(mem_allocator allocator) {
  memtbl_handle mthandle = mem_alloc(allocator, sizeof(memtbl));
  if (!mem_is_valid(mthandle)) return (memtbl_handle){0};
  memtbl *tblp = mem_p(mthandle);
  tblp->mem_map_handle = map_create(allocator);
  if (!mem_is_valid(tblp->mem_map_handle)) {
    mem_free(mthandle);
    return (memtbl_handle){0};
  }
  return mthandle;
}

bool memtbl_is_valid(memtbl_handle mthandle) {
  return (mem_is_valid(mthandle) &&
          map_is_valid(((memtbl *)mem_p(mthandle))->mem_map_handle));
}

void memtbl_destroy(memtbl_handle mthandle) {
  if (!memtbl_is_valid(mthandle)) return;
  memtbl *mtp = mem_p(mthandle);
  map_iter it = map_first_value_iter(mtp->mem_map_handle);
  while (!map_iter_done(it)) {
    mem_free(map_iter_value(it));
    it = map_next_value_iter(it);
  }
  map_destroy(mtp->mem_map_handle);
  mem_free(mthandle);
}

static mem_handle memtbl_alloc(mem_allocator allocator, size_t size) {
  memtbl *tblp = allocator.allocator_data;
  if (!tblp || !mem_is_valid(tblp->mem_map_handle)) return (mem_handle){0};
  void *data = malloc(size);
  if (!data) return (mem_handle){0};
  mem_handle result = {.allocator = allocator, .data = data, .size = size};
  bool mapset_success = map_set(tblp->mem_map_handle, data, result);
  if (!mapset_success) {
    free(data);
    return (mem_handle){0};
  }
  return result;
}

static mem_handle memtbl_realloc(mem_handle handle, size_t size) {
  if (!mem_is_valid(handle)) return (mem_handle){0};
  memtbl *tblp = handle.allocator.allocator_data;
  if (!tblp || !mem_is_valid(tblp->mem_map_handle)) return (mem_handle){0};
  void *old_data = mem_p(handle);
  void *new_data = realloc(old_data, size);
  if (!new_data) return (mem_handle){0};
  mem_handle result = {
      .allocator = handle.allocator, .data = new_data, .size = size};
  // (old_data may be free'd by realloc but we only use it as a hash key here.
  // Static analyzers will complain.)
  if (new_data != old_data) {
    if (!map_set(tblp->mem_map_handle, new_data, result) ||
        !map_delete(tblp->mem_map_handle, old_data)) {
      free(new_data);
      return (mem_handle){0};
    }
  }
  return result;
}

static mem_handle memtbl_free(mem_handle handle) {
  if (!mem_is_valid(handle)) return (mem_handle){0};
  memtbl *tblp = handle.allocator.allocator_data;
  if (!tblp || !mem_is_valid(tblp->mem_map_handle)) return (mem_handle){0};
  void *data = mem_p(handle);
  map_delete(tblp->mem_map_handle, data);
  free(data);
  return (mem_handle){0};
}

static void *memtbl_p(mem_handle handle) {
  if (!mem_is_valid(handle)) return (void *)0;
  memtbl *tblp = handle.allocator.allocator_data;
  if (!tblp || !mem_is_valid(tblp->mem_map_handle)) return (void *)0;
  mem_handle result = map_get(tblp->mem_map_handle, handle.data);
  // Don't use mem_p here! result = handle if it's in the table, or is invalid
  // if it's not.
  return result.data;
}

static const mem_allocator_spec MEMTBL_ALLOCATOR_SPEC =
    (mem_allocator_spec){.allocator_type = MEM_ALLOCATOR_TYPE_MEMTBL,
                         .alloc_func = memtbl_alloc,
                         .realloc_func = memtbl_realloc,
                         .free_func = memtbl_free,
                         .p_func = memtbl_p};

inline mem_allocator mem_allocator_memtbl(memtbl_handle mth) {
  return (mem_allocator){.allocator_spec = &MEMTBL_ALLOCATOR_SPEC,
                         .allocator_data = mth.data};
}
