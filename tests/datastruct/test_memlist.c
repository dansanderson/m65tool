#include <signal.h>

#include "datastruct/memlist.h"
#include "unity.h"

static memlist MEMLIST_INVALID = (memlist){0};
static memlist_handle MEMLIST_HANDLE_INVALID = (memlist_handle){0};

memlist ml;
void dummy_sighandler(int i) {}
void (*orig_handler)(int);

void setUp(void) {
  ml = memlist_create();
  orig_handler = signal(SIGINT, dummy_sighandler);
}

void tearDown(void) {
  memlist_destroy(&ml);
  signal(SIGINT, orig_handler);
}

void test_MemlistCreate_IsValid(void) {
  TEST_ASSERT_TRUE(memlist_is_valid(&ml));
}

void test_MemlistAlloc_IsValid(void) {
  memlist_handle mlh = memlist_alloc(&ml, 128);
  TEST_ASSERT_TRUE(memlist_handle_is_valid(mlh));
}

void test_MemlistAlloc_PreservesSigintHandler(void) {
  memlist_handle mlh = memlist_alloc(&ml, 128);
  void (*handler)(int) = signal(SIGINT, dummy_sighandler);
  TEST_ASSERT_EQUAL(dummy_sighandler, handler);
}

void test_MemlistAlloc_InvalidList_IsInvalid(void) {
  memlist_handle mlh = memlist_alloc(&MEMLIST_INVALID, 128);
  TEST_ASSERT_FALSE(memlist_handle_is_valid(mlh));
}

void test_MemlistRealloc_IsValid(void) {
  memlist_handle mlh = memlist_alloc(&ml, 128);
  TEST_ASSERT_TRUE(memlist_handle_is_valid(mlh));
  memlist_handle reply = memlist_realloc(mlh, 256);
  TEST_ASSERT_TRUE(memlist_handle_is_valid(mlh));
  TEST_ASSERT_TRUE(memlist_handle_is_valid(reply));
  TEST_ASSERT_EQUAL(mlh.id, reply.id);
  TEST_ASSERT_EQUAL(mlh.mlp, reply.mlp);
}

void test_MemlistRealloc_PreservesSigintHandler(void) {
  memlist_handle mlh = memlist_alloc(&ml, 128);
  memlist_realloc(mlh, 256);
  void (*handler)(int) = signal(SIGINT, dummy_sighandler);
  TEST_ASSERT_EQUAL(dummy_sighandler, handler);
}

void test_MemlistFreeOne_MakesHandlerInvalid(void) {
  memlist_handle mlh = memlist_alloc(&ml, 128);
  TEST_ASSERT_TRUE(memlist_handle_is_valid(mlh));
  memlist_free_one(mlh);
  TEST_ASSERT_FALSE(memlist_handle_is_valid(mlh));
  memlist_free_one(mlh);
  TEST_ASSERT_FALSE(memlist_handle_is_valid(mlh));
}

void test_MemlistFreeAll_MakesHandlersInvalid(void) {
  memlist_handle mlh1 = memlist_alloc(&ml, 128);
  TEST_ASSERT_TRUE(memlist_handle_is_valid(mlh1));
  memlist_handle mlh2 = memlist_alloc(&ml, 128);
  TEST_ASSERT_TRUE(memlist_handle_is_valid(mlh2));
  memlist_handle mlh3 = memlist_alloc(&ml, 128);
  TEST_ASSERT_TRUE(memlist_handle_is_valid(mlh3));
  memlist_free_all(&ml);
  TEST_ASSERT_FALSE(memlist_handle_is_valid(mlh1));
  TEST_ASSERT_FALSE(memlist_handle_is_valid(mlh2));
  TEST_ASSERT_FALSE(memlist_handle_is_valid(mlh3));
}

void test_MemlistDestroy_MakesListInvalid(void) {
  memlist_destroy(&ml);
  TEST_ASSERT_FALSE(memlist_is_valid(&ml));
}
