#include "str.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memlist.h"

static const size_t STR_CSTR_BUFSIZE = 1024;
static char STR_CSTR_BUFFER[STR_CSTR_BUFSIZE];
static const str STR_INVALID = (str){0};
static const strbuf STRBUF_INVALID = (strbuf){0};

str str_from_cstr(const char *cstr) {
  return (str){
      .value = cstr, .length = strlen((const char *)cstr), .allocated = false};
}

str str_from_bytes(const char *bytes, size_t length) {
  return (str){.value = bytes, .length = length, .allocated = false};
}

static str do_str_duplicate(const char *value, size_t length, memlist *mlp) {
  char *mem;
  memlist_handle mlh = (memlist_handle){0};
  if (mlp != (void *)0) {
    mlh = memlist_alloc(mlp, length);
    if (!memlist_handle_is_valid(mlh)) return STR_INVALID;
    mem = memlist_P(mlh);
  } else {
    mem = malloc(sizeof(char) * length);
    if (!mem) return STR_INVALID;
  }
  memcpy(mem, value, length);
  return (str){.value = mem, .length = length, .allocated = true, .mlh = mlh};
}

str str_duplicate_cstr(const char *cstr, memlist *mlp) {
  if (!cstr) {
    return STR_INVALID;
  }
  size_t length = strlen((const char *)cstr);
  return do_str_duplicate(cstr, length, mlp);
}

str str_duplicate_str(str strval, memlist *mlp) {
  if (!str_is_valid(strval)) {
    return STR_INVALID;
  }
  return do_str_duplicate(strval.value, strval.length, mlp);
}

str str_duplicate_strbuf(strbuf bufval, memlist *mlp) {
  if (!strbuf_is_valid(bufval)) {
    return STR_INVALID;
  }
  return do_str_duplicate(bufval.value, bufval.length, mlp);
}

void str_destroy(str *strp) {
  if (!str_is_valid(*strp)) {
    return;
  }
  if (strp->allocated) {
    if (memlist_handle_is_valid(strp->mlh)) {
      memlist_free_one(strp->mlh);
    } else {
      free((void *)strp->value);
    }
    strp->allocated = false;
  }
  strp->value = (void *)0;
  strp->length = 0;
}

str str_write_cstr_to_buf(str strval, char *buf, size_t bufsize) {
  if (!str_is_valid(strval) || buf == (char *)0) {
    return STR_INVALID;
  }
  int i = 0;
  while (i < bufsize - 1 && i < strval.length) {
    buf[i] = strval.value[i];
    ++i;
  }
  buf[i] = (char)0;
  return (str){.value = buf, .length = i, .allocated = false};
}

char *str_cstr(str strval) {
  if (!str_is_valid(strval)) {
    return (void *)0;
  }
  str_write_cstr_to_buf(strval, STR_CSTR_BUFFER, STR_CSTR_BUFSIZE);
  return STR_CSTR_BUFFER;
}

bool str_is_valid(str strval) {
  return strval.value != (void *)0;
}

size_t str_length(str strval) {
  return strval.length;
}

int str_find(str strval, str substring) {
  if (!str_is_valid(strval) || !str_is_valid(substring)) return -1;
  if (strval.length < substring.length) return -1;

  for (int start_i = 0; start_i < strval.length - substring.length + 1;
       start_i++) {
    int offset = 0;
    while (offset < substring.length &&
           strval.value[start_i + offset] == substring.value[offset])
      offset++;
    if (offset == substring.length) return start_i;
  }
  return -1;
}

int str_compare(str first, str second) {
  if (!str_is_valid(first) && !str_is_valid(second)) return 0;
  if (!str_is_valid(first)) return -1;
  if (!str_is_valid(second)) return 1;

  unsigned int min_length =
      first.length < second.length ? first.length : second.length;
  int i = 0;
  while (i < min_length && first.value[i] == second.value[i]) i++;
  if (i == first.length) {
    if (first.length == second.length) return 0;
    return -1;
  } else if (i == second.length) {
    return 1;
  } else if (first.value[i] < second.value[i]) {
    return -1;
  }
  return 1;
}

str str_split_pop(str strval, str delim, str *part) {
  int pos = str_find(strval, delim);
  part->value = strval.value;
  part->allocated = false;
  if (pos == -1) {
    part->length = strval.length;
    return STR_INVALID;
  } else {
    part->length = pos;
    return (str){.value = strval.value + pos + delim.length,
                 .length = strval.length - pos - delim.length,
                 .allocated = false};
  }
}

strbuf strbuf_create(size_t size) {
  char *mem = malloc(sizeof(char) * size);
  if (!mem) {
    return STRBUF_INVALID;
  }
  return (strbuf){.value = mem, .length = 0, .bufsize = size};
}

strbuf strbuf_create_to_memlist(size_t size, memlist *mlp) {
  memlist_handle mlh = memlist_alloc(mlp, sizeof(char) * size);
  if (!memlist_handle_is_valid(mlh)) return STRBUF_INVALID;
  return (strbuf){
      .value = memlist_P(mlh), .length = 0, .bufsize = size, .mlh = mlh};
}

void strbuf_destroy(strbuf *bufvalp) {
  if (!strbuf_is_valid(*bufvalp)) return;
  if (bufvalp->mlh.mlp) {
    // (Test the memlist pointer, not handle validity, to know if this was
    // created with a memlist. Handle goes "invalid" when freed.)
    memlist_free_one(bufvalp->mlh);
  } else {
    free((void *)bufvalp->value);
  }
  bufvalp->value = (void *)0;
  bufvalp->length = 0;
  bufvalp->bufsize = 0;
}

bool strbuf_is_valid(strbuf bufval) {
  if (bufval.mlh.mlp) {
    return memlist_handle_is_valid(bufval.mlh);
  }
  return bufval.value != (void *)0;
}

strbuf strbuf_duplicate(strbuf bufval) {
  if (!strbuf_is_valid(bufval)) return STRBUF_INVALID;
  strbuf newbuf = strbuf_create(bufval.bufsize);
  memcpy(newbuf.value, bufval.value, bufval.bufsize);
  newbuf.length = bufval.length;
  newbuf.bufsize = bufval.bufsize;
  return newbuf;
}

static bool grow_strbuf(strbuf *buf) {
  size_t newsize = buf->bufsize * 2;
  buf->value = realloc(buf->value, newsize);
  buf->bufsize = newsize;
  return (!!buf->value);
}

static bool do_strbuf_concatenate(strbuf *destbuf, const char *cstr,
                                  size_t length) {
  if (!destbuf || !strbuf_is_valid(*destbuf)) return false;
  while (destbuf->length + length > destbuf->bufsize) {
    if (!grow_strbuf(destbuf)) return false;
  }
  for (int i = 0; i < length; i++) {
    destbuf->value[destbuf->length + i] = cstr[i];
  }
  destbuf->length += length;
  return true;
}

bool strbuf_concatenate_cstr(strbuf *destbuf, const char *cstr) {
  if (!cstr) return false;
  return do_strbuf_concatenate(destbuf, cstr, strlen(cstr));
}

bool strbuf_concatenate_str(strbuf *destbuf, str strval) {
  if (!str_is_valid(strval)) return false;
  return do_strbuf_concatenate(destbuf, strval.value, strval.length);
}

bool strbuf_concatenate_strbuf(strbuf *destbuf, strbuf sourcebuf) {
  if (!strbuf_is_valid(sourcebuf)) return false;
  return do_strbuf_concatenate(destbuf, sourcebuf.value, sourcebuf.length);
}

bool strbuf_concatenate_printf(strbuf *destbuf, const char *fmt, ...) {
  va_list v;
  size_t length;

  if (!destbuf || !strbuf_is_valid(*destbuf) || !fmt) return false;

  va_start(v, fmt);
  length = vsnprintf(NULL, 0, fmt, v) + 1;
  va_end(v);

  while (destbuf->length + length > destbuf->bufsize) {
    if (!grow_strbuf(destbuf)) return false;
  }

  va_start(v, fmt);
  vsnprintf(destbuf->value + destbuf->length, length, fmt, v);
  va_end(v);
  destbuf->length += length - 1;
  return true;
}
