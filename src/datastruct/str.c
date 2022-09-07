#include "str.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mem.h"

static const size_t STR_CSTR_BUFSIZE = 1024;
static char STR_CSTR_BUFFER[STR_CSTR_BUFSIZE];
static const str STR_INVALID = (str){0};
static const strbuf STRBUF_INVALID = (strbuf){0};

str str_from_cstr(const char *cstr) {
  return mem_handle_from_ptr(cstr, strlen(cstr));
}

str str_duplicate_strbuf(strbuf bufval) {
  mem_handle hdl = bufval.data;
  hdl.size = bufval.length;
  return mem_duplicate(hdl);
}

str str_duplicate_str(str strval) {
  return mem_duplicate(strval);
}

str str_duplicate_cstr_with_allocator(const char *cstr,
                                      mem_allocator allocator) {
  if (!cstr) {
    return STR_INVALID;
  }
  mem_handle hdl = str_from_cstr(cstr);
  return (str)mem_duplicate_with_allocator(allocator, hdl);
}

str str_duplicate_str_with_allocator(str strval, mem_allocator allocator) {
  return mem_duplicate_with_allocator(allocator, strval);
}

str str_duplicate_strbuf_with_allocator(strbuf bufval,
                                        mem_allocator allocator) {
  mem_handle hdl = bufval.data;
  hdl.size = bufval.length;
  return mem_duplicate_with_allocator(allocator, hdl);
}

inline void str_destroy(str strval) {
  mem_free(strval);
}

str str_write_cstr_to_buf(str strval, char *buf, size_t bufsize) {
  if (!str_is_valid(strval) || buf == (char *)0) {
    return STR_INVALID;
  }
  int i = 0;
  while (i < bufsize - 1 && i < strval.size) {
    buf[i] = ((char *)mem_p(strval))[i];
    ++i;
  }
  buf[i] = (char)0;
  return (str){
      .info.plain_info.ptr = buf, .size = i, .allocator = MEM_NOT_ALLOCATED};
}

char *str_cstr(str strval) {
  if (!str_is_valid(strval)) {
    return (void *)0;
  }
  str_write_cstr_to_buf(strval, STR_CSTR_BUFFER, STR_CSTR_BUFSIZE);
  return STR_CSTR_BUFFER;
}

inline bool str_is_valid(str strval) {
  return mem_is_valid(strval);
}

inline size_t str_length(str strval) {
  return strval.size;
}

int str_find(str strval, str substring) {
  if (!str_is_valid(strval) || !str_is_valid(substring)) return -1;
  if (strval.size < substring.size) return -1;

  char *strval_p = mem_p(strval);
  char *substring_p = mem_p(substring);

  for (int start_i = 0; start_i < strval.size - substring.size + 1; start_i++) {
    int offset = 0;
    while (offset < substring.size &&
           strval_p[start_i + offset] == substring_p[offset])
      offset++;
    if (offset == substring.size) return start_i;
  }
  return -1;
}

int str_compare(str first, str second) {
  if (!str_is_valid(first) && !str_is_valid(second)) return 0;
  if (!str_is_valid(first)) return -1;
  if (!str_is_valid(second)) return 1;

  char *first_p = mem_p(first);
  char *second_p = mem_p(second);

  unsigned int min_length = first.size < second.size ? first.size : second.size;
  int i = 0;
  while (i < min_length && first_p[i] == second_p[i]) i++;
  if (i == first.size) {
    if (first.size == second.size) return 0;
    return -1;
  } else if (i == second.size) {
    return 1;
  } else if (first_p[i] < second_p[i]) {
    return -1;
  }
  return 1;
}

str str_split_pop(str strval, str delim, str *part) {
  int pos = str_find(strval, delim);
  char *strval_p = mem_p(strval);

  part->info.plain_info.ptr = strval_p;
  part->allocator = MEM_NOT_ALLOCATED;
  if (pos == -1) {
    part->size = strval.size;
    return STR_INVALID;
  } else {
    part->size = pos;
    return (str){.info.plain_info.ptr = strval_p + pos + delim.size,
                 .size = strval.size - pos - delim.size,
                 .allocator = MEM_NOT_ALLOCATED};
  }
}

strbuf_handle strbuf_create(mem_allocator allocator, size_t size) {
  mem_handle bufhdl = mem_alloc(allocator, sizeof(strbuf));
  if (!mem_is_valid(bufhdl)) return (strbuf_handle){0};
  mem_handle data = mem_alloc_clear(allocator, size);
  if (!mem_is_valid(data)) return (strbuf_handle){0};

  strbuf *bufp = mem_p(bufhdl);
  bufp->data = data;
  bufp->length = 0;
  return bufhdl;
}

void strbuf_destroy(strbuf_handle buf_handle) {
  if (!mem_is_valid(buf_handle)) return;
  strbuf *bufp = mem_p(buf_handle);
  mem_free(bufp->data);
  mem_free(buf_handle);
}

bool strbuf_is_valid(strbuf_handle buf_handle) {
  return (mem_is_valid(buf_handle) &&
          mem_is_valid(((strbuf *)mem_p(buf_handle))->data));
}

strbuf_handle strbuf_duplicate(strbuf_handle buf_handle) {
  if (!strbuf_is_valid(buf_handle)) return (strbuf_handle){0};

  strbuf *bufp = mem_p(buf_handle);
  strbuf_handle new_handle =
      strbuf_create(buf_handle.allocator, bufp->data.size);
  if (!strbuf_is_valid(new_handle)) return (strbuf_handle){0};
  strbuf *newp = mem_p(new_handle);
  memcpy(mem_p(newp->data), mem_p(bufp->data), bufp->data.size);
  newp->length = bufp->length;

  return new_handle;
}

static bool grow_strbuf(strbuf_handle buf_handle) {
  strbuf *bufp = mem_p(buf_handle);
  size_t newsize = bufp->data.size * 2;
  bufp->data = mem_realloc(bufp->data, newsize);
  return mem_is_valid(bufp->data);
}

static bool do_strbuf_concatenate(strbuf_handle buf_handle, const char *cstr,
                                  size_t length) {
  if (!strbuf_is_valid(buf_handle)) return false;
  strbuf *destbufp = mem_p(buf_handle);

  while (destbufp->length + length > destbufp->data.size) {
    if (!grow_strbuf(buf_handle)) return false;
  }
  for (int i = 0; i < length; i++) {
    ((char *)mem_p(destbufp->data))[destbufp->length + i] = cstr[i];
  }
  destbufp->length += length;
  return true;
}

bool strbuf_concatenate_cstr(strbuf_handle buf_handle, const char *cstr) {
  if (!cstr) return false;
  return do_strbuf_concatenate(buf_handle, cstr, strlen(cstr));
}

bool strbuf_concatenate_str(strbuf_handle buf_handle, str strval) {
  if (!str_is_valid(strval)) return false;
  return do_strbuf_concatenate(buf_handle, mem_p(strval), strval.size);
}

bool strbuf_concatenate_strbuf(strbuf_handle buf_handle,
                               strbuf_handle sourcebuf_handle) {
  if (!strbuf_is_valid(sourcebuf_handle)) return false;
  strbuf *sourcebufp = mem_p(sourcebuf_handle);
  return do_strbuf_concatenate(buf_handle, mem_p(sourcebufp->data),
                               sourcebufp->length);
}

bool strbuf_concatenate_printf(strbuf_handle buf_handle, const char *fmt, ...) {
  va_list v;
  size_t length;

  if (!strbuf_is_valid(buf_handle) || !fmt) return false;
  strbuf *destbufp = mem_p(buf_handle);

  va_start(v, fmt);
  length = vsnprintf(NULL, 0, fmt, v) + 1;
  va_end(v);

  while (destbufp->length + length > destbufp->data.size) {
    if (!grow_strbuf(buf_handle)) return false;
  }

  va_start(v, fmt);
  vsnprintf(mem_p(destbufp->data) + destbufp->length, length, fmt, v);
  va_end(v);
  destbufp->length += length - 1;
  return true;
}
