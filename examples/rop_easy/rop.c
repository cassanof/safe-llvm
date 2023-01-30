#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int *value = NULL;

int set_value(void) {
  char resp[2] = "y";
  if (value != NULL) {
    *value = 0xfedefede;
    return 0;
  }
  printf("value is NULL - %s\n", resp);
  return 1;
}

void vulnerable(char *string) {
  char buffer[100];
  strcpy(buffer, string);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: %s <string>", argv[0]);
    exit(1);
  }
  value = (int *)malloc(sizeof(int));
  *value = 0;
  vulnerable(argv[1]);
  printf("value = %x - should be '0xfedefede'\n", *value);
  return 0;
}
