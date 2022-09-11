#include <signal.h>

#include "datastruct/memtbl.h"
#include "unity.h"

memtbl_handle mth;
mem_allocator ma;

void dummy_sigint(int sig) {}

void setUp(void) {
  mth = memtbl_create(MEM_ALLOCATOR_PLAIN);
  ma = mem_allocator_memtbl(mth);
}

void tearDown(void) {
  memtbl_destroy(mth);
}

void test_MemAlloc_MemtblAllocator_AllocatesMemory(void) {
  mem_handle result = mem_alloc(ma, sizeof(int));
  TEST_ASSERT_TRUE(mem_is_valid(result));
  int *ptr = mem_p(result);
  *ptr = 123;
  TEST_ASSERT_EQUAL(123, *ptr);
  result = mem_free(result);
  TEST_ASSERT_FALSE(mem_is_valid(result));
}

void test_MemAlloc_MemtblAllocator_RestoresSigintHandler(void) {
  signal(SIGINT, dummy_sigint);
  mem_handle result = mem_alloc(ma, sizeof(int));
  result = mem_free(result);
  void (*prev_handler)(int) = signal(SIGINT, SIG_DFL);
  TEST_ASSERT_EQUAL_PTR(dummy_sigint, prev_handler);
}

void test_MemAllocClear_MemtblAllocator_AllocatesClearMemory(void) {
  mem_handle result = mem_alloc_clear(ma, sizeof(char[10]));
  TEST_ASSERT_TRUE(mem_is_valid(result));
  char *arr = mem_p(result);
  for (int i = 0; i < 10; i++) {
    TEST_ASSERT_EQUAL(0, arr[i]);
  }
  result = mem_free(result);
  TEST_ASSERT_FALSE(mem_is_valid(result));
}

void test_MemRealloc_MemtblAllocator_ReallocatesMemory(void) {
  mem_handle result = mem_alloc_clear(ma, sizeof(char[10]));
  TEST_ASSERT_TRUE(mem_is_valid(result));
  TEST_ASSERT_EQUAL(10, mem_size(result));
  result = mem_realloc(result, sizeof(char[20]));
  TEST_ASSERT_TRUE(mem_is_valid(result));
  TEST_ASSERT_EQUAL(20, mem_size(result));
  result = mem_free(result);
  TEST_ASSERT_FALSE(mem_is_valid(result));
}

void test_MemRealloc_MemtblAllocator_RestoresSigintHandler(void) {
  signal(SIGINT, dummy_sigint);
  mem_handle result = mem_alloc(ma, sizeof(int));
  result = mem_realloc(result, sizeof(char));
  result = mem_free(result);
  void (*prev_handler)(int) = signal(SIGINT, SIG_DFL);
  TEST_ASSERT_EQUAL_PTR(dummy_sigint, prev_handler);
}

void test_MemP_MemtblAllocatorFreedMemory_ReturnsNullPointer(void) {
  mem_handle result = mem_alloc(ma, sizeof(int));
  result = mem_free(result);
  TEST_ASSERT_EQUAL_PTR((void *)0, mem_p(result));
}

void test_MemDuplicate_MemtblAllocatedHandle_ReturnsMemtblDuplicate(void) {
  mem_handle result = mem_alloc(ma, sizeof(char[10]));
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
