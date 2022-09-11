#include <stdio.h>

#include "datastruct/map.h"
#include "datastruct/str.h"
#include "unity.h"

map_handle maph;
mem_handle val, val2;
str strkey;
void *ptrkey;

void setUp(void) {
  strkey = str_from_cstr("key1");
  ptrkey = &strkey;
  val = mem_alloc(MEM_ALLOCATOR_PLAIN, sizeof(int));
  val2 = mem_alloc(MEM_ALLOCATOR_PLAIN, sizeof(int));
  maph = map_create(MEM_ALLOCATOR_PLAIN);
}

void tearDown(void) {
  mem_free(val);
  mem_free(val2);
  map_destroy(maph);
}

void test_MapGet_StrKey_FindsValue(void) {
  TEST_ASSERT_TRUE(map_set(maph, strkey, val));
  mem_handle result = map_get(maph, strkey);
  TEST_ASSERT_TRUE(mem_is_valid(result));
  TEST_ASSERT_EQUAL_PTR(mem_p(val), mem_p(result));
}

void test_MapGet_PtrKey_FindsValue(void) {
  TEST_ASSERT_TRUE(map_set(maph, ptrkey, val));
  mem_handle result = map_get(maph, ptrkey);
  TEST_ASSERT_TRUE(mem_is_valid(result));
  TEST_ASSERT_EQUAL_PTR(mem_p(val), mem_p(result));
}

void test_MapGet_UnsetStrKey_ReturnsInvalidHandle(void) {
  mem_handle result = map_get(maph, strkey);
  TEST_ASSERT_FALSE(mem_is_valid(result));
}

void test_MapGet_UnsetPtrKey_ReturnsInvalidHandle(void) {
  mem_handle result = map_get(maph, ptrkey);
  TEST_ASSERT_FALSE(mem_is_valid(result));
}

void test_MapSet_ExistingKey_ReplacesValue(void) {
  TEST_ASSERT_TRUE(map_set(maph, strkey, val));
  mem_handle result = map_get(maph, strkey);
  TEST_ASSERT_TRUE(mem_is_valid(result));
  TEST_ASSERT_EQUAL_PTR(mem_p(val), mem_p(result));
  TEST_ASSERT_TRUE(map_set(maph, strkey, val2));
  result = map_get(maph, strkey);
  TEST_ASSERT_TRUE(mem_is_valid(result));
  TEST_ASSERT_EQUAL_PTR(mem_p(val2), mem_p(result));
}

void test_MapSet_17Keys_GrowsTable(void) {
  char loc, *locptr = &loc;

  map *mapptr = mem_p(maph);
  TEST_ASSERT_EQUAL(0, mapptr->entry_count);
  TEST_ASSERT_EQUAL(32, mapptr->table_size);

  for (int i = 0; i < 17; i++) {
    TEST_ASSERT_TRUE(map_set(maph, (void *)locptr + i, val));
  }

  TEST_ASSERT_EQUAL(17, mapptr->entry_count);
  TEST_ASSERT_EQUAL(64, mapptr->table_size);
}

void test_MapDelete_ExistingStrKey_UnsetsKey(void) {
  TEST_ASSERT_TRUE(map_set(maph, strkey, val));
  mem_handle result = map_get(maph, strkey);
  TEST_ASSERT_TRUE(mem_is_valid(result));

  // Duplicate key to ensure it's using the string contents and not the address.
  str another_strkey =
      str_duplicate_cstr_with_allocator("key1", MEM_ALLOCATOR_PLAIN);
  TEST_ASSERT_TRUE(str_is_valid(another_strkey));
  TEST_ASSERT_TRUE(map_delete(maph, another_strkey));
  str_destroy(another_strkey);

  result = map_get(maph, strkey);
  TEST_ASSERT_FALSE(mem_is_valid(result));
}

void test_MapDelete_ExistingPtrKey_UnsetsKey(void) {
  TEST_ASSERT_TRUE(map_set(maph, ptrkey, val));
  mem_handle result = map_get(maph, ptrkey);
  TEST_ASSERT_TRUE(mem_is_valid(result));

  TEST_ASSERT_TRUE(map_delete(maph, ptrkey));

  result = map_get(maph, ptrkey);
  TEST_ASSERT_FALSE(mem_is_valid(result));
}

void test_MapDelete_UnsetStrKey_ReturnsFalse(void) {
  TEST_ASSERT_FALSE(map_delete(maph, ptrkey));
  TEST_ASSERT_FALSE(map_delete(maph, strkey));
}

void test_MapDelete_AfterGrowth_ShrinksTable(void) {
  char loc, *locptr = &loc;

  map *mapptr = mem_p(maph);
  TEST_ASSERT_EQUAL(0, mapptr->entry_count);
  TEST_ASSERT_EQUAL(32, mapptr->table_size);

  for (int i = 0; i < 17; i++) {
    TEST_ASSERT_TRUE(map_set(maph, (void *)locptr + i, val));
  }

  TEST_ASSERT_EQUAL(17, mapptr->entry_count);
  TEST_ASSERT_EQUAL(64, mapptr->table_size);

  TEST_ASSERT_TRUE(map_delete(maph, (void *)locptr));
  TEST_ASSERT_TRUE(map_delete(maph, (void *)locptr + 1));
  TEST_ASSERT_EQUAL(15, mapptr->entry_count);
  TEST_ASSERT_EQUAL(32, mapptr->table_size);
}

void test_MapIter_NonEmptyMap_ReturnsAllValues(void) {
  TEST_ASSERT_TRUE(map_set(maph, strkey, val));
  TEST_ASSERT_TRUE(map_set(maph, str_from_cstr("key2"), val2));
  bool saw_val1 = false, saw_val2 = false;
  map_iter it = map_first_value_iter(maph);
  while (!map_iter_done(it)) {
    void *itvalp = mem_p(map_iter_value(it));
    if (itvalp == mem_p(val)) {
      saw_val1 = true;
    } else if (itvalp == mem_p(val2)) {
      saw_val2 = true;
    } else {
      TEST_FAIL_MESSAGE("Map iterator had unexpected value");
    }
    it = map_next_value_iter(it);
  }
  TEST_ASSERT_TRUE(saw_val1);
  TEST_ASSERT_TRUE(saw_val2);
}

void test_MapIter_EmptyMap_ReturnsDoneIteratorFirst(void) {
  TEST_ASSERT_TRUE(map_iter_done(map_first_value_iter(maph)));
}
