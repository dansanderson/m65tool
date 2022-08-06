#include <config.h>
#include <stdio.h>

#include "examplemod.h"

int main(void) {
  puts("Hello world!\n");
  puts("This is " PACKAGE_STRING ".\n");
  examplemod_print_all_messages();
  return 0;
}
