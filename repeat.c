#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
  if (argc < 3) {
    printf("usage: %s <num> <string|char>\n", argv[0]);
    return 1;
  }

  int num = atoi(argv[1]);

  if (num < 1 || strlen(argv[2]) < 1) {
    return 1;
  }

  for (int i = 0; i < num; ++i) {
    printf("%s", argv[2]);
  }
  putchar('\n');

  return 0;
}

