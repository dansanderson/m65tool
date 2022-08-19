#include "datastruct/datastruct.h"
#include "unity.h"

void setUp(void) {}

void tearDown(void) {}

void test_DatastructFunc_ReturnsZero(void) {
  TEST_ASSERT_EQUAL_MESSAGE(0, datastruct_func(999), "func returns 0");
}
