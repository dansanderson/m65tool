#include "examplemod/examplemod.h"
#include "exampletwo/mocks/mock_exampletwo.h"
#include "unity.h"

void setUp(void) {}

void tearDown(void) {}

void test_ExamplemodPrintAllMessages_DoesOk(void) {
  examplemod_print_all_messages();

  // TEST_FAIL_MESSAGE("This is a test that fails.");
  TEST_MESSAGE("Yup it works");
}

void test_Square_UsesExampleTwo(void) {
  exampletwo_mult_ExpectAndReturn(7, 7, 49);
  int result = examplemod_square(7);
  TEST_ASSERT_EQUAL_INT(49, result);
}
