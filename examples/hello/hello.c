#include <stdio.h>
#include <stdlib.h>



void hello() {
  printf("Hello, world!");
}

int main(void) {
  for (int i = 0; i < 10; i++) {
    hello();
  }
  return EXIT_SUCCESS;
}

