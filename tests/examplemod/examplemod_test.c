#include "examplemod.h"

#include <stdio.h>

void setUp(void) {
  puts("Called examplemod_test setUp\n");
}

void tearDown(void) {
  puts("Called examplemod_test tearDown\n");
}

void test_ExamplemodPrintAllMessages_DoesOk(void) {
  examplemod_print_all_messages();
}
