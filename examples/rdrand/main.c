#include <stdint.h>
#include <stdio.h>

// program that tests intel's RDRAND instruction
int main(int argc, char *argv[]) {
  uint32_t rand;
  __asm__ __volatile__("rdrand %0" : "=r"(rand));
  printf("RDRAND returned %u\n", rand);


  // rdseed
  uint32_t seed;
  __asm__ __volatile__("rdseed %0" : "=r"(seed));
  printf("RDSEED returned %u\n", seed);

  return 0;
}
