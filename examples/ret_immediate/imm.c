#include <stdio.h>

int random(int _garbage) {
  int x = _garbage + 0xC2; // immediatelly encodes a ret when unaligned

  int y = _garbage + 0x12C2; // also encodes a ret

  int z = _garbage + 0x12C223; // should not encode a ret

  int a = _garbage + 0xC223; // should not encode a ret
  printf("x = %d. y = %d. z = %d. a = %d. garbage = %d\n", x, y, z, a, _garbage);
  return x;
}

int main(int argc, char *argv[])
{
  random(0xdeadbeef);
  return 0;
}
