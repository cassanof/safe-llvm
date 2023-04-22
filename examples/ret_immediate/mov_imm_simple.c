#include <stdio.h>

int random(int _garbage) {
  int sum = 0;
  for (int i = 0; i < 100; i++) {
    sum += i + _garbage;
  }

  int x = 0xC2; // immediatelly encodes a ret when unaligned
  return x;
}

int main(int argc, char *argv[]) {
  random(0xdeadbeef);
  printf("expected:\n");
  printf("x = 194. y = 4802. z = 1229347. a = 49699. garbage = -559038737\n");
  return 0;
}
