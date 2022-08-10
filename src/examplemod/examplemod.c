#include "examplemod.h"

#include <stdio.h>

#include "exampletwo/exampletwo.h"
#include "priv1.h"

void print_another(void);

void examplemod_print_all_messages(void) {
  puts("** examplemod print_all_messages start\n");
  priv1_print();
  print_another();

#ifdef LINUX
  puts("Host OS detected: Linux\n");
#elif WINDOWS
  puts("Host OS detected: Windows\n");
#elif APPLE
  puts("Host OS detected: macOS\n");
#else
  puts("Host OS *not* detected ??\n");
#endif

  puts("** examplemod print_all_messages end\n");
}

void print_another(void) {
  puts("print_another()");
}

int examplemod_square(int a) {
  return exampletwo_mult(a, a);
}
