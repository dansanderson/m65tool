#include "datastruct/strmap.h"
#include "unity.h"

memlist ml;
strmap map;

static const strmap INVALID_STRMAP = (strmap){0};

void setUp(void) {
  ml = memlist_create();
}

void tearDown(void) {
  memlist_destroy(&ml);
  strmap_destroy(&map);
}

void test_StrmapCreate_IsValid(void) {
  strmap map = strmap_create();
  TEST_ASSERT_TRUE(strmap_is_valid(map));
  strmap_destroy(&map);
  TEST_ASSERT_FALSE(strmap_is_valid(map));
}

void test_StrmapCreateToMemlist_IsValid(void) {
  strmap map = strmap_create_to_memlist(&ml);
  TEST_ASSERT_TRUE(strmap_is_valid(map));
  strmap_destroy(&map);
  TEST_ASSERT_FALSE(strmap_is_valid(map));
}

void test_StrmapSet_NewKey_CanGet(void) {
  int alpha_val = 1;
  int beta_val = 2;
  int gamma_val = 3;
  strmap map = strmap_create();
  TEST_ASSERT_TRUE(strmap_set(&map, str_from_cstr("alpha"), &alpha_val));
  TEST_ASSERT_TRUE(strmap_set(&map, str_from_cstr("beta"), &beta_val));
  TEST_ASSERT_TRUE(strmap_set(&map, str_from_cstr("gamma"), &gamma_val));

  // Use str_duplicate to ensure key text is read from different memory.
  str alpha_key = str_duplicate("alpha");
  int *result_alpha = strmap_get(&map, alpha_key);
  str_destroy(&alpha_key);
  TEST_ASSERT_EQUAL(alpha_val, *result_alpha);
  str beta_key = str_duplicate("beta");
  int *result_beta = strmap_get(&map, str_from_cstr("beta"));
  str_destroy(&beta_key);
  TEST_ASSERT_EQUAL(beta_val, *result_beta);
  str gamma_key = str_duplicate("gamma");
  int *result_gamma = strmap_get(&map, str_from_cstr("gamma"));
  str_destroy(&gamma_key);
  TEST_ASSERT_EQUAL(gamma_val, *result_gamma);
}

void test_StrmapSet_OverwriteAllocatedKey_Deallocates(void) {}

void test_StrmapSet_ManyKeys_GrowsTable(void) {}
void test_StrmapSetCopy_CanGet(void) {}
void test_StrmapSetCopy_ToMemlist_UsesMemlist(void) {}
