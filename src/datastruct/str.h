/**
 * @file str.h
 * @brief String and string buffer routines.
 *
 * A str is a mem_handle. It can refer to a region of characters in memory,
 * with or without a null terminator. It can also track a dynamically allocated
 * string. Unlike a C string, a str can contain nulls, and has O(1) access to
 * its length. The str value can be passed by value to functions, and returned
 * by value from functions, regardless of the length of the string.
 *
 * str character memory is owned by the caller by default. If the str is created
 * from existing memory, such as a C string via `str_from_cstr`, the str points
 * to the memory. If the memory is freed, the str must be discarded by the
 * caller, or invalidated by calling `str_destroy`.
 *
 * `str_duplicate` allocates memory. To free this memory, call `str_destroy`.
 */

#ifndef DATASTRUCT_STR_H
#define DATASTRUCT_STR_H

#include <stdbool.h>
#include <stdlib.h>

#include "mem.h"

/**
 * @brief A string reference.
 */
typedef mem_handle str;

/**
 * @brief A string buffer reference.
 */
typedef mem_handle strbuf_handle;

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
  // The allocated buffer
  mem_handle data;

  // Length of the stored string value in bytes
  size_t length;
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

// clang-format off
/**
 * @brief Duplicates a str or strbuf into a new str, reusing the allocator.
 *
 * @param v The str or strbuf
 * @return str The new allocated str
 */
#define str_duplicate(v) \
  _Generic((v), \
    str: str_duplicate_str, \
    strbuf_handle: str_duplicate_strbuf \
  )((v), (void *)0)
// clang-format on
str str_duplicate_str(str strval);
str str_duplicate_strbuf(strbuf_handle buf_handle);

// clang-format off
/**
 * @brief Duplicates a C string, str, or strbuf into a new str, with the given allocator.
 *
 * @param v The str or strbuf
 * @param allocator The mem_allocator to use
 * @return str The new allocated str
 */
#define str_duplicate_with_allocator(v, allocator) \
  _Generic((v), \
    char *: str_duplicate_cstr_with_allocator, \
    str: str_duplicate_str_with_allocator, \
    strbuf_handle: str_duplicate_strbuf_with_allocator \
  )((v), (allocator))
// clang-format on
str str_duplicate_cstr_with_allocator(const char *cstr,
                                      mem_allocator allocator);
str str_duplicate_str_with_allocator(str strval, mem_allocator allocator);
str str_duplicate_strbuf_with_allocator(strbuf_handle buf_handle,
                                        mem_allocator allocator);

/**
 * @brief Invalidate and deallocate a str, as appropriate.
 *
 * This only attempts to deallocate memory if the str was created by
 * `str_duplicate`. In all cases, it updates the str to be invalid.
 *
 * @param strp Ptr to the str
 */
void str_destroy(str strval);

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
 * @param allocator The mem_allocator to use
 * @param size A suggested initial buffer size, in characters
 * @return strbuf_handle A reference to the strbuf, or invalid
 */
strbuf_handle strbuf_create(mem_allocator allocator, size_t size);

/**
 * @brief Destroys a strbuf.
 *
 * @param bufval Ptr to the strbuf to destroy
 */
void strbuf_destroy(strbuf_handle buf_handle);

/**
 * @return true if the strbuf is valid
 */
bool strbuf_is_valid(strbuf_handle buf_handle);

/**
 * @brief Gets a str reference for the current contents of the buffer.
 *
 * The result points to buffer memory directly. To get an allocated str copy of
 * the buffer contents, use `str_duplicate(buffer)`.
 *
 * @param buf_handle The string buffer
 * @return str A str reference to the buffer contents
 */
str strbuf_str(strbuf_handle buf_handle);

// clang-format off
/**
 * @brief Concatenates text to the end of the string buffer.
 *
 * The value can be a C string, a str, or another strbuf.
 *
 * This may reallocate the buffer if the value outgrows its size. If the
 * reallocation fails, this returns an invalid strbuf.
 *
 * @param buf_handle Handle for the strbuf
 * @param v The C string (null-terminated char *), str, or strbuf_handle to concatenate
 * @return true on success
 */
#define strbuf_concatenate(buf_handle, v) \
  _Generic((v), \
    char *: strbuf_concatenate_cstr, \
    str: strbuf_concatenate_str, \
    strbuf_handle: strbuf_concatenate_strbuf \
    )((buf_handle), (v))
// clang-format on
bool strbuf_concatenate_cstr(strbuf_handle buf_handle, const char *cstr);
bool strbuf_concatenate_str(strbuf_handle buf_handle, str strval);
bool strbuf_concatenate_strbuf(strbuf_handle buf_handle,
                               strbuf_handle sourcebuf_handle);

/**
 * @brief Append string-formatted data to the strbuf.
 *
 * This is similar to `sprintf`, with protection against overflow and no
 * truncation.
 *
 * @param buf_handle Handle for the strbuf
 * @param fmt The printf format string
 * @param ... Arguments to the format string
 * @return true on success
 */
bool strbuf_concatenate_printf(strbuf_handle buf_handle, const char *fmt, ...);

/**
 * @brief Allocates a new strbuf with the contents of the given strbuf.
 *
 * The caller is responsible for calling `strbuf_destroy` on the result when
 * no longer needed.
 *
 * @param buf_handle Handle for the strbuf to duplicate
 * @return strbuf The new strbuf
 */
strbuf_handle strbuf_duplicate(strbuf_handle buf_handle);

#endif
