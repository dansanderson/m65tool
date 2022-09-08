#include "mem.h"

#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"

const mem_allocator MEM_NOT_ALLOCATED =
    (mem_allocator){.allocator_type = MEM_ALLOCATOR_TYPE_NOT_ALLOCATED};
const mem_allocator MEM_ALLOCATOR_PLAIN =
    (mem_allocator){.allocator_type = MEM_ALLOCATOR_TYPE_PLAIN};

/**
 * @brief Macro to disable SIGINT for a block.
 *
 * Usage: sigint_guard { ...statements... }
 *
 * Do not exit prematurely out of the block (return, goto). This will result in
 * the SIGINT handler not getting restored correctly.
 */
// This macro treats the statement block as a run-once for loop, storing the
// current SIGINT handler and replacing it with SIG_IGN at the beginning of the
// block, then restoring the SIGINT handler at the end.
//
// TODO: Does this need to use a custom SIGINT handler to trigger the current
// handler at the end of the block if tripped during the block?
#define sigint_guard                           \
  for (struct {                                \
         void (*sigint_handler)(int);          \
         int i;                                \
       } guard = {signal(SIGINT, SIG_IGN), 0}; \
       !guard.i; (guard.i = 1, signal(SIGINT, guard.sigint_handler)))

inline mem_handle mem_handle_from_ptr(void *ptr, size_t size) {
  return (mem_handle){.allocator_type = MEM_ALLOCATOR_TYPE_NOT_ALLOCATED,
                      .info.plain.ptr = ptr,
                      .size = size};
}

/**
 * @returns a `mem_allocator` that uses a given memtbl.
 */
inline mem_allocator mem_allocator_memtbl(memtbl_handle mth) {
  return (mem_allocator){.allocator_type = MEM_ALLOCATOR_TYPE_MEMTBL,
                         .info.memtbl.handle = mth};
}

mem_handle mem_alloc(mem_allocator ma, size_t size) {
  if (ma.allocator_type == MEM_ALLOCATOR_TYPE_INVALID) return (mem_handle){0};
  switch (ma.allocator_type) {
    case MEM_ALLOCATOR_TYPE_MEMTBL:
      if (!memtbl_is_valid(ma.info.memtbl.handle)) return (mem_handle){0};
      memtbl *mtp = mem_p(ma.info.memtbl.handle);

      memtbl_id id;
      bool set_result = false;
      sigint_guard {
        void *ptr = malloc(size);
        mem_handle internal_hdl =
            (mem_handle){.allocator_type = MEM_ALLOCATOR_TYPE_PLAIN,
                         .info.plain.ptr = ptr,
                         .size = size};
        id = mtp->next_id;
        set_result = map_set(mtp->mem_map_handle, id, internal_hdl);
        mtp->next_id++;
      }
      if (!set_result) return (mem_handle){0};

      return (mem_handle){.allocator_type = MEM_ALLOCATOR_TYPE_MEMTBL,
                          .info.memtbl.ptr = mtp,
                          .info.memtbl.id = id,
                          .size = size};

    case MEM_ALLOCATOR_TYPE_PLAIN:
      void *ptr = malloc(size);
      return (mem_handle){.allocator_type = MEM_ALLOCATOR_TYPE_PLAIN,
                          .info.plain.ptr = ptr,
                          .size = size};

    default:
      // It is an error to attempt this with other allocator types.
      return (mem_handle){0};
  }
}

mem_handle mem_alloc_clear(mem_allocator ma, size_t size) {
  mem_handle handle = mem_alloc(ma, size);
  if (mem_is_valid(handle)) {
    memset(mem_p(handle), 0, size);
  }
  return handle;
}

mem_handle mem_realloc(mem_handle handle, size_t size) {
  switch (handle.allocator_type) {
    case MEM_ALLOCATOR_TYPE_MEMTBL:
      memtbl *mtp = handle.info.memtbl.ptr;

      bool set_result = false;
      sigint_guard {
        mem_handle internal_hdl =
            map_get(mtp->mem_map_handle, handle.info.memtbl_info.id);
        void *ptr = realloc(mem_p(internal_hdl), size);
        internal_hdl.info.plain.ptr = ptr;
        set_result = map_set(mtp->mem_map_handle, handle.info.memtbl_info.id,
                             internal_hdl);
      }
      if (!set_result) return (mem_handle){0};

      handle.size = size;
      return handle;

    case MEM_ALLOCATOR_TYPE_PLAIN:
      void *origptr = mem_p(handle);
      void *ptr = realloc(origptr, size);
      return (mem_handle){.allocator_type = MEM_ALLOCATOR_TYPE_PLAIN,
                          .info.plain.ptr = ptr,
                          .size = size};

    default:
      // It is an error to attempt this with other allocator types.
      return (mem_handle){0};
  }
}

void mem_free(mem_handle handle) {
  void *ptr = mem_p(handle);
  if (!ptr) return;

  switch (handle.allocator_type) {
    case MEM_ALLOCATOR_TYPE_MEMTBL:
      memtbl *mtp = handle.info.memtbl.ptr;

      sigint_guard {
        free(ptr);
        map_delete(mtp->mem_map_handle, handle.info.memtbl_info.id);
      }
      break;

    case MEM_ALLOCATOR_TYPE_PLAIN:
      free(ptr);
      break;

    default:
      // Freeing a handle from other allocator types does nothing.
  }
}

void *mem_p(mem_handle handle) {
  switch (handle.allocator_type) {
    case MEM_ALLOCATOR_TYPE_MEMTBL:
      memtbl *mtp = handle.info.memtbl.ptr;
      return map_get(mtp->mem_map_handle, handle.info.memtbl_info.id);

    case MEM_ALLOCATOR_TYPE_PLAIN:
    case MEM_ALLOCATOR_TYPE_NOT_ALLOCATED:
      return handle.info.plain.ptr;

    default:
      return (void *)0;
  }
}

inline bool mem_is_valid(mem_handle handle) {
  return (!mem_p(handle));
}

mem_handle mem_duplicate_with_allocator(mem_allocator allocator,
                                        mem_handle handle) {
  if (!mem_is_valid(handle)) return (mem_handle){0};
  mem_handle new_handle = mem_alloc(allocator, handle.size);
  if (!mem_is_valid(new_handle)) return (mem_handle){0};
  memcpy(mem_p(new_handle), mem_p(handle), handle.size);
  return new_handle;
}

mem_handle mem_duplicate(mem_handle handle) {
  mem_allocator allocator =
      (mem_allocator){.allocator_type = handle.allocator_type};
  if (handle.allocator_type == MEM_ALLOCATOR_TYPE_MEMTBL) {
    // Reconstructing the mem_allocator from a memtbl * drops information about
    // how the memtbl itself was allocated. This is necessary because the
    // handle being duplicated can't contain a complete handle to memtbl.
    allocator.info.memtbl.handle =
        mem_handle_from_ptr(handle.info.memtbl.ptr, sizeof(memtbl));
  }

  return mem_duplicate_with_allocator(allocator, handle);
}

memtbl_handle memtbl_create(mem_allocator ma) {
  memtbl_handle mth = mem_alloc(ma, sizeof(memtbl));
  memtbl *mtp = mem_p(mth);
  if (!mtp) return (memtbl_handle){0};
  mtp->next_id = 0;
  mtp->mem_map_handle = map_create(ma);
  if (!mem_p(mtp->mem_map_handle)) {
    mem_free(mth);
    return (memtbl_handle){0};
  }
  return mth;
}

bool memtbl_is_valid(memtbl_handle mth) {
  memtbl *mtp = mem_p(mth);
  return (mtp != (void *)0 && map_is_valid(mtp->mem_map_handle));
}

void memtbl_destroy(memtbl_handle mth) {
  memtbl *mtp = mem_p(mth);
  if (mtp) {
    map_iter it = map_first_value_iter(mtp->mem_map_handle);
    while (!map_iter_done(it)) {
      mem_free(it.value_handle);
      it = map_next_value_iter(it);
    }
    map_destroy(mtp->mem_map_handle);
  }
}