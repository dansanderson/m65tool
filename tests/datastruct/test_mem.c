#include <string.h>

#include "datastruct/mem.h"
#include "unity.h"

void test_MemHandleFromPtr_RepresentsMemory(void) {
  int foo = 0;
  mem_handle result = mem_handle_from_ptr(&foo, sizeof(int));
  TEST_ASSERT_TRUE(mem_is_valid(result));
  TEST_ASSERT_EQUAL_PTR(&foo, mem_p(result));
}

void test_MemAlloc_PlainAllocator_AllocatesMemory(void) {
  mem_handle result = mem_alloc(MEM_ALLOCATOR_PLAIN, sizeof(int));
  TEST_ASSERT_TRUE(mem_is_valid(result));
  int *ptr = mem_p(result);
  *ptr = 123;
  TEST_ASSERT_EQUAL(123, *ptr);
  result = mem_free(result);
  TEST_ASSERT_FALSE(mem_is_valid(result));
}

void test_MemAlloc_InvalidAllocator_ReturnsInvalidMemHandle(void) {
  mem_handle result = mem_alloc((mem_allocator){0}, sizeof(int));
  TEST_ASSERT_FALSE(mem_is_valid(result));
}

void test_MemAllocClear_PlainAllocator_AllocatesClearMemory(void) {
  mem_handle result = mem_alloc_clear(MEM_ALLOCATOR_PLAIN, sizeof(char[10]));
  TEST_ASSERT_TRUE(mem_is_valid(result));
  char *arr = mem_p(result);
  for (int i = 0; i < 10; i++) {
    TEST_ASSERT_EQUAL(0, arr[i]);
  }
  result = mem_free(result);
  TEST_ASSERT_FALSE(mem_is_valid(result));
}

void test_MemAllocClear_InvalidAllocator_ReturnsInvalidMemHandle(void) {
  mem_handle result = mem_alloc_clear((mem_allocator){0}, sizeof(int));
  TEST_ASSERT_FALSE(mem_is_valid(result));
}

void test_MemRealloc_PlainAllocator_ReallocatesMemory(void) {
  mem_handle result = mem_alloc_clear(MEM_ALLOCATOR_PLAIN, sizeof(char[10]));
  TEST_ASSERT_TRUE(mem_is_valid(result));
  TEST_ASSERT_EQUAL(10, mem_size(result));
  result = mem_realloc(result, sizeof(char[20]));
  TEST_ASSERT_TRUE(mem_is_valid(result));
  TEST_ASSERT_EQUAL(20, mem_size(result));
  result = mem_free(result);
  TEST_ASSERT_FALSE(mem_is_valid(result));
}

void test_MemRealloc_InvalidHandle_ReturnsInvalidHandle(void) {
  mem_handle result = mem_realloc((mem_handle){0}, sizeof(char[20]));
  TEST_ASSERT_FALSE(mem_is_valid(result));
}

void test_MemP_InvalidHandle_ReturnsNullPointer(void) {
  TEST_ASSERT_EQUAL_PTR((void *)0, mem_p((mem_handle){0}));
}

void test_MemDuplicate_PlainAllocatedHandle_ReturnsPlainDuplicate(void) {
  mem_handle result = mem_alloc(MEM_ALLOCATOR_PLAIN, sizeof(char[10]));
  TEST_ASSERT_TRUE(mem_is_valid(result));
  char *arr = mem_p(result);
  for (int i = 0; i < 10; i++) {
    arr[i] = i;
  }
  mem_handle new_mem = mem_duplicate(result);
  TEST_ASSERT_TRUE(mem_is_valid(new_mem));
  char *new_arr = mem_p(new_mem);
  TEST_ASSERT_EQUAL_CHAR_ARRAY(arr, new_arr, 10);
  result = mem_free(result);
  TEST_ASSERT_FALSE(mem_is_valid(result));
  new_mem = mem_free(new_mem);
  TEST_ASSERT_FALSE(mem_is_valid(new_mem));
}

void test_MemDuplicateWithAllocator_FromPtr_ReturnsAllocatedHandle(void) {
  const char strval[] = "Hello there";
  // (Purposefully ignoring the null terminator.)
  mem_handle result = mem_handle_from_ptr((void *)strval, strlen(strval));
  TEST_ASSERT_TRUE(mem_is_valid(result));
  mem_handle new_mem =
      mem_duplicate_with_allocator(MEM_ALLOCATOR_PLAIN, result);
  TEST_ASSERT_TRUE(mem_is_valid(new_mem));
  char *new_str = mem_p(new_mem);
  TEST_ASSERT_EQUAL_CHAR_ARRAY(strval, new_str, strlen(strval));

  // (Freeing from-ptr memory should do nothing, but still return invalid
  // handle.)
  result = mem_free(result);
  TEST_ASSERT_FALSE(mem_is_valid(result));
  new_mem = mem_free(new_mem);
  TEST_ASSERT_FALSE(mem_is_valid(new_mem));
}
