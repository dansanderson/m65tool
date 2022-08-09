#include <config.h>
#include <stdio.h>

#include "examplemod/examplemod.h"

int main(void) {
  puts("Hello world!\n");
  puts("This is " PACKAGE_STRING ".\n");
  examplemod_print_all_messages();
  printf("square of 11 is %d\n", examplemod_square(11));
  return 0;
}
