#include <stdint.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  __asm__("ret");
  __asm__("ret $0x8");
  __asm__("iret");

  uint64_t very_long_variable_name = 0x1234567890abcdef;

  printf("Hello, rets!\n");

  return 0;
}
