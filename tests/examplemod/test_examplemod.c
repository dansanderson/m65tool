#include <stdio.h>

#include "examplemod/examplemod.h"
#include "unity.h"

void setUp(void) {}

void tearDown(void) {}

void test_ExamplemodPrintAllMessages_DoesOk(void) {
  examplemod_print_all_messages();

  // TEST_FAIL_MESSAGE("This is a test that fails.");
}
