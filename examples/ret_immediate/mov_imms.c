#include <stdio.h>

int random(int _garbage) {
  int x = 0xC2; // immediatelly encodes a ret when unaligned

  int y = 0x12C2; // also encodes a ret

  int z = 0x12C223; // should not encode a ret

  int a = 0xC223; // should not encode a ret
  printf("x = %d. y = %d. z = %d. a = %d. garbage = %d\n", x, y, z, a,
         _garbage);
  return x;
}

int main(int argc, char *argv[]) {
  random(0xdeadbeef);
  printf("expected:\n");
  printf("x = 194. y = 4802. z = 1229347. a = 49699. garbage = -559038737\n");
  return 0;
}
