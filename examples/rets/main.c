#include <stdio.h>

int main(int argc, char *argv[]) {
  __asm__ ("ret");
  __asm__ ("ret $0x8");
  __asm__ ("retf");
  __asm__ ("retf $0x8");
  __asm__ ("iret");

  printf("Hello, rets!\n");

  return 0;
}
