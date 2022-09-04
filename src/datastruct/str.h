#ifndef DATASTRUCT_STR_H
#define DATASTRUCT_STR_H

#include <stdbool.h>
#include <stdlib.h>

#include "memlist.h"

/**
 * @brief A string reference.
 *
 * Unlike a C string, a str can contain nulls, and has O(1) access to its
 * length. The str value can be passed by value to functions, and returned
 * by value from functions, regardless of the length of the string.
 *
 * A str can be valid or invalid. Use `str_is_valid()` to test. str functions
 * will fail gracefully when given an invalid str (such as by returning an
 * invalid str), to support chaining of functions.
 *
 * str character memory is owned by the caller by default. If the str is created
 * from existing memory, such as a C string via `str_from_cstr`, the str points
 * to the memory. If the memory is freed, the str must be discarded by the
 * caller, or invalidated by calling `str_destroy`.
 *
 * `str_duplicate` allocates memory. To free this memory, call `str_destroy`.
 */
typedef struct str {
  // Ptr to first char, or (void *)0 if invalid
  const char *value;

  // Length of string value in bytes
  size_t length;

  // Whether the memory can be freed by `str_destroy`
  bool allocated;

  // A handle to a memlist entry, if allocated on a memlist.
  memlist_handle mlh;
} str;

/**
 * @brief A string buffer.
 *
 * A strbuf is allocated dynamically and grows automatically. Use it to build
 * strings. It can be memory storage for str references.
 *
 * A strbuf can be valid or invalid. Use `strbuf_is_valid()` to test. strbuf
 * functions will fail gracefully when given an invalid strbuf (such as by
 * returning an invalid strbuf), to support chaining of functions.
 */
typedef struct strbuf {
  // Ptr to allocated buffer
  char *value;

  // Length of the stored string value in bytes
  size_t length;

  // Size of the allocated buffer
  size_t bufsize;

  // A handle to a memlist entry, if allocated on a memlist.
  memlist_handle mlh;
} strbuf;

/**
 * @brief Makes a str that points to a given C string.
 *
 * The resulting str refers to the C string memory. To copy the C string into
 * newly allocated memory, use a strbuf.
 *
 * @param cstr A null-terminated C string
 * @return str A str populated with the C string
 */
str str_from_cstr(const char *cstr);

/**
 * @brief Makes a str that points to a given set of bytes.
 *
 * The bytes do *not* need to be null-terminated, and may contain inner null
 * values.
 *
 * The resulting str refers to the bytes memory. To copy the bytes into newly
 * allocated memory, use a strbuf.
 *
 * @param bytes The first address of the bytes
 * @param length The number of bytes to include in the str
 * @return str A str populated with the bytes
 */
str str_from_bytes(const char *bytes, size_t length);

// clang-format off
#define str_duplicate(v) \
  _Generic((v), \
    char *: str_duplicate_cstr, \
    str: str_duplicate_str, \
    strbuf: str_duplicate_strbuf \
  )((v), (void *)0)

#define str_duplicate_to_memlist(v, mlp) \
  _Generic((v), \
    char *: str_duplicate_cstr, \
    str: str_duplicate_str, \
    strbuf: str_duplicate_strbuf \
  )((v), (mlp))
// clang-format on

/**
 * @brief Allocates memory and copies a null-terminated C string.
 *
 * Prefer `str_duplicate(V)` or `str_duplicate_to_memlist(V, memlist *)` macros
 * over calling directly. These macros accepts a C string, a str, or a strbuf
 * for V.
 *
 * Call `str_destroy` to deallocate.
 *
 * @param strval The str value to duplicate
 * @param mlp Ptr to a memlist for allocation, or (void *)0 to use malloc
 * @return str A str of the duplicate
 */
str str_duplicate_cstr(const char *cstr, memlist *mlp);

/**
 * @brief Allocates memory and copies a str value.
 *
 * See `str_duplicate_cstr`.
 *
 * @param strval The str value to duplicate
 * @param mlp Ptr to a memlist for allocation, or (void *)0 to use malloc
 * @return str A str of the duplicate
 */
str str_duplicate_str(str strval, memlist *mlp);

/**
 * @brief Allocates memory and copies a strbuf value to a str.
 *
 * See `str_duplicate_cstr`.
 *
 * @param bufval The strbuf to duplicate
 * @param mlp Ptr to a memlist for allocation, or (void *)0 to use malloc
 * @return str A str of the duplicate
 */
str str_duplicate_strbuf(strbuf bufval, memlist *mlp);

/**
 * @brief Invalidate and deallocate a str, as appropriate.
 *
 * This only attempts to deallocate memory if the str was created by
 * `str_duplicate`. In all cases, it updates the str to be invalid.
 *
 * @param strp Ptr to the str
 */
void str_destroy(str *strp);

/**
 * @brief Copies str data to a C string buffer, with a null terminator.
 *
 * This copies at most bufsize-1 characters. It returns the actual number of
 * characters written, not including the null terminator.
 *
 * If the str is empty or invalid, this writes only the null terminator to the
 * buffer.
 *
 * This returns a new str describing the data written to the buffer. If the
 * strval argument is invalid, so is this return value.
 *
 * @param strval The str whose data to copy
 * @param buf Ptr to the first position to write
 * @param bufsize The maximum number of bytes to write
 * @return str A str of the copied string
 */
str str_write_cstr_to_buf(str strval, char *buf, size_t bufsize);

/**
 * @brief Copies a str to a global buffer as a null terminated C string.
 *
 * This is a convenience for briefly needing a str as a null terminated C
 * string, such as for use with `printf`.
 *
 * This is only a little bit convenient! Take care not to use this more than
 * once in a `printf` argument list. It will always return a pointer to the
 * same buffer, so all printed values will be the last value written to the
 * buffer.
 *
 * The global buffer is a fixed size. This will never overrun the global
 * buffer, and always adds a null terminator. If you need a different sized
 * buffer, allocate your own, then use `str_write_cstr_to_buf`.
 *
 * Each call overwrites the buffer. Be sure to use the value before the next
 * call to `str_to_cstr_buffer`.
 *
 * @param strval The str to copy
 * @return char* The null-terminated string in the global buffer
 */
char *str_cstr(str strval);

/**
 * @return true if the str is valid
 */
bool str_is_valid(str strval);

/**
 * @return size_t The length of the str
 */
size_t str_length(str strval);

/**
 * @brief Finds the left-most occurrence of a substring in a string.
 *
 * @param strval The str to be searched
 * @param substring The substring to locate inside str
 * @return int The character index of the located occurrence of
 *   substring, or -1 if not found
 */
int str_find(str strval, str substring);

/**
 * @brief Compares two strs lexicographically.
 *
 * This returns -1 if first < second, 0 if first == second, or 1 if first >
 * second.
 *
 * Unlike strcmp, this does not stop at a null character, but instead compares
 * all characters of the shorter string length.
 *
 * An invalid string is less than a valid string. Two invalid strings are
 * equal. (You shouldn't be using invalid strings. This is for completeness.)
 *
 * @param first The first str
 * @param second The second str
 * @return int -1, 0, or 1 representing how first relates to second
 */
int str_compare(str first, str second);

/**
 * @brief Splits a str with a delimiter and returns the next part.
 *
 * The found part is placed in the `part` output parameter. It may be an empty
 * string if strval contains only delim. This returns the str that begins after
 * the delimiter, or an invalid str if the delim is not found.
 *
 * To iterate over the parts of strval separated by delim:
 *
 *   str strval = str_from_cstr("one two three");
 *   str delim = str_from_cstr(" ");
 *   while (str_is_valid(strval)) {
 *     str part;
 *     strval = str_split_pop(strval, delim, &part);
 *     printf("Found: '%s'\n", str_to_cstr_buffer(part));
 *   }
 *
 * This prints:
 *   Found: 'one'
 *   Found: 'two'
 *   Found: 'three'
 *
 * If strval is "one two three ", this prints:
 *   Found: 'one'
 *   Found: 'two'
 *   Found: 'three'
 *   Found: ''
 *
 * If strval is "one  two three", this prints:
 *   Found: 'one'
 *   Found: ''
 *   Found: 'two'
 *   Found: 'three'
 *
 * @param strval The str to split
 * @param delim The delimiter, as a str
 * @param[out] part The str up to the delimiter
 * @return str The str that starts after the delimiter to the end of strval
 */
str str_split_pop(str strval, str delim, str *part);

/**
 * @brief Creates a strbuf.
 *
 * Use `strbuf_is_valid` to confirm that memory was allocated correctly. strbuf
 * functions will fail gracefully if a strbuf is invalid.
 *
 * @param size A suggested initial buffer size, in characters
 * @return strbuf The strbuf
 */
strbuf strbuf_create(size_t size);

/**
 * @brief Creates a strbuf.
 *
 * Use `strbuf_is_valid` to confirm that memory was allocated correctly. strbuf
 * functions will fail gracefully if a strbuf is invalid.
 *
 * @param size A suggested initial buffer size, in characters
 * @param mlp Ptr to a memlist for allocation
 * @return strbuf The strbuf
 */
strbuf strbuf_create_to_memlist(size_t size, memlist *mlp);

/**
 * @brief Destroys a strbuf.
 *
 * @param bufval Ptr to the strbuf to destroy
 */
void strbuf_destroy(strbuf *bufvalp);

/**
 * @return true if the strbuf is valid
 */
bool strbuf_is_valid(strbuf bufval);

/**
 * @brief Allocates a new strbuf with the contents of the given strbuf.
 *
 * The caller is responsible for calling `strbuf_destroy` on the result when no
 * longer needed.
 *
 * @param bufval The strbuf to duplicate
 * @return strbuf The new strbuf
 */
strbuf strbuf_duplicate(strbuf bufval);

// clang-format off
#define strbuf_concatenate(destbufp, v) \
  _Generic((v), \
    char *: strbuf_concatenate_cstr, \
    str: strbuf_concatenate_str, \
    strbuf: strbuf_concatenate_strbuf \
    )((destbufp), (v))
// clang-format on

/**
 * @brief Concatenates a C string to the end of the string buffer.
 *
 * This may reallocate the buffer if the value outgrows its size. If the
 * reallocation fails, this returns an invalid strbuf.
 *
 * The generic macro strbuf_concatenate(destbuf, v) accepts a C string, a str,
 * or a strbuf as v.
 *
 * @param destbuf Ptr to the string buffer
 * @param cstr The null-terminated C string to concatenate
 * @return true on success
 */
bool strbuf_concatenate_cstr(strbuf *destbuf, const char *cstr);

/**
 * @brief Concatenates a str to the end of the string buffer.
 *
 * See `strbuf_concatenate_cstr`.
 */
bool strbuf_concatenate_str(strbuf *destbuf, str strval);

/**
 * @brief Concatenates the contents of sourcebuf to the end of destbuf.
 *
 * See `strbuf_concatenate_cstr`.
 */
bool strbuf_concatenate_strbuf(strbuf *destbuf, strbuf sourcebuf);

/**
 * @brief Append string-formatted data to the strbuf.
 *
 * This is similar to `sprintf`, with protection against overflow and no
 * truncation.
 *
 * @param destbuf Ptr to the string buffer
 * @param fmt The printf format string
 * @param ... Arguments to the format string
 * @return true on success
 */
bool strbuf_concatenate_printf(strbuf *destbuf, const char *fmt, ...);

#endif
