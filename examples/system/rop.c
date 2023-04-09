#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void gadgets(void) { 
  __asm__("mov %rax, (%rdx); ret");
  __asm__("pop %rdx; ret");
  __asm__("pop %rax; ret");
  __asm__("pop %rdi; ret");
}

void vulnerable(char *string) {
  char buffer[100];
  memcpy(buffer, string, 400);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: %s <string>", argv[0]);
    exit(1);
  }

  // just a casual system call
  system("echo 'Hello, world!'");

  // casually call vulnerable
  vulnerable(argv[1]);
  return 0;
}
