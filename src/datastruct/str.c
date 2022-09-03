#include "str.h"

#include <stdbool.h>
#include <string.h>

static const size_t STR_CSTR_BUFSIZE = 1024;
static char STR_CSTR_BUFFER[STR_CSTR_BUFSIZE];
static const str STR_INVALID =
    (str){.value = (void *)0, .length = 0, .allocated = false};

str str_from_cstr(const char *cstr) {
  return (str){
      .value = cstr, .length = strlen((const char *)cstr), .allocated = false};
}

str str_from_bytes(const char *bytes, size_t length) {
  return (str){.value = bytes, .length = length, .allocated = false};
}

str str_duplicate_cstr(const char *cstr) {
  if (!cstr) {
    return STR_INVALID;
  }
  size_t length = strlen((const char *)cstr);
  char *mem = malloc(length);
  if (!mem) {
    return STR_INVALID;
  }
  memcpy(mem, cstr, length);
  return (str){.value = mem, .length = length, .allocated = true};
}

str str_duplicate_str(str strval) {
  if (!str_is_valid(strval)) {
    return STR_INVALID;
  }
  char *mem = malloc(strval.length);
  if (!mem) {
    return STR_INVALID;
  }
  memcpy(mem, strval.value, strval.length);
  return (str){.value = mem, .length = strval.length, .allocated = true};
}

str str_duplicate_strbuf(strbuf bufval) {
  if (!strbuf_is_valid(bufval)) {
    return STR_INVALID;
  }
  char *mem = malloc(bufval.length);
  if (!mem) {
    return STR_INVALID;
  }
  memcpy(mem, bufval.value, bufval.length);
  return (str){.value = mem, .length = bufval.length, .allocated = true};
}

void str_destroy(str *strp) {
  if (!str_is_valid(*strp)) {
    return;
  }
  if (strp->allocated) {
    free((void *)strp->value);
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

bool strbuf_is_valid(strbuf bufval) {
  return bufval.value != (void *)0;
}
