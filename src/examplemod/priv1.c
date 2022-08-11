#include "priv1.h"

#include <stdio.h>

void _examplemod_priv1_print(void) {
  int i = 123;
  printf("examplemod priv1: %d\n", i);
}

int _examplemod_priv1_add(int a, int b) {
  return a + b;
}
