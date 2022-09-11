#include "mem.h"

#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void *mem_handle_data(mem_handle handle) {
  return handle.data;
}

static const mem_allocator_spec MEM_ALLOCATOR_NOT_ALLOCATED_SPEC =
    (mem_allocator_spec){.allocator_type = MEM_ALLOCATOR_TYPE_NOT_ALLOCATED,
                         .p_func = mem_handle_data};

const mem_allocator MEM_ALLOCATOR_NOT_ALLOCATED =
    (mem_allocator){.allocator_spec = &MEM_ALLOCATOR_NOT_ALLOCATED_SPEC};

static mem_handle plain_alloc(mem_allocator allocator, size_t size) {
  return (mem_handle){
      .data = malloc(size), .size = size, .allocator = allocator};
}

static mem_handle plain_realloc(mem_handle handle, size_t size) {
  if (!mem_is_valid(handle)) return (mem_handle){0};
  return (mem_handle){.data = realloc(handle.data, size), .size = size};
}

static mem_handle plain_free(mem_handle handle) {
  free(handle.data);
  return (mem_handle){0};
}

static const mem_allocator_spec MEM_ALLOCATOR_PLAIN_SPEC =
    (mem_allocator_spec){.allocator_type = MEM_ALLOCATOR_TYPE_PLAIN,
                         .alloc_func = plain_alloc,
                         .realloc_func = plain_realloc,
                         .free_func = plain_free,
                         .p_func = mem_handle_data};

const mem_allocator MEM_ALLOCATOR_PLAIN =
    (mem_allocator){.allocator_spec = &MEM_ALLOCATOR_PLAIN_SPEC};

inline mem_handle mem_handle_from_ptr(void *ptr, size_t size) {
  return (mem_handle){
      .allocator = MEM_ALLOCATOR_NOT_ALLOCATED, .data = ptr, .size = size};
}

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

mem_handle mem_alloc(mem_allocator allocator, size_t size) {
  if (!allocator.allocator_spec || !allocator.allocator_spec->alloc_func)
    return (mem_handle){0};
  mem_handle result;
  sigint_guard {
    result = allocator.allocator_spec->alloc_func(allocator, size);
  }
  return result;
}

mem_handle mem_alloc_clear(mem_allocator allocator, size_t size) {
  mem_handle handle = mem_alloc(allocator, size);
  if (mem_is_valid(handle)) {
    memset(mem_p(handle), 0, size);
  }
  return handle;
}

mem_handle mem_realloc(mem_handle handle, size_t size) {
  if (!mem_is_valid(handle) || !handle.allocator.allocator_spec ||
      !handle.allocator.allocator_spec->realloc_func)
    return (mem_handle){0};
  mem_handle result;
  sigint_guard {
    result = handle.allocator.allocator_spec->realloc_func(handle, size);
  }
  return result;
}

mem_handle mem_free(mem_handle handle) {
  if (!mem_is_valid(handle) || !handle.allocator.allocator_spec ||
      !handle.allocator.allocator_spec->free_func)
    return (mem_handle){0};
  mem_handle result;
  sigint_guard {
    result = handle.allocator.allocator_spec->free_func(handle);
  }
  return result;
}

void *mem_p(mem_handle handle) {
  if (!mem_is_valid(handle) || !handle.allocator.allocator_spec ||
      !handle.allocator.allocator_spec->p_func)
    return (void *)0;
  return handle.allocator.allocator_spec->p_func(handle);
}

inline size_t mem_size(mem_handle handle) {
  return handle.size;
}

inline bool mem_is_valid(mem_handle handle) {
  return handle.data != (void *)0;
}

mem_handle mem_duplicate_with_allocator(mem_allocator allocator,
                                        mem_handle handle) {
  if (!mem_is_valid(handle)) return (mem_handle){0};
  mem_handle new_handle = mem_alloc(allocator, handle.size);
  if (!mem_is_valid(new_handle)) return (mem_handle){0};
  memcpy(mem_p(new_handle), mem_p(handle), handle.size);
  return new_handle;
}

inline mem_handle mem_duplicate(mem_handle handle) {
  return mem_duplicate_with_allocator(handle.allocator, handle);
}
