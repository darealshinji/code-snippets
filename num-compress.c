/* simple and fast method to compress a large amount of (only) digits (like Pi) */

#include <stdio.h>
#include <string.h>

void num_comp()
{
  int num = -1;
  char c;

  while ((c = fgetc(stdin)) != EOF) {
    if (c < '0' || c > '9') {
      continue;
    }

    int diff = c - '0';

    if (num < 0) {
      if (c == '0') {
        putchar(diff);
      } else {
        num = diff;
      }
    } else if (num < 10) {
      num = num * 10 + diff;
      if (num > 25) {
        putchar(num);
        num = -1;
      }
    } else {
      int old = num;
      num = num * 10 + diff;
      if (num > 255) {
        putchar(old);
        num = diff;
      } else {
        putchar(num);
        num = -1;
      }
    }
  }
  putchar(num);
}

void num_decomp()
{
  unsigned char ch[1];

  while (fread(ch, 1, 1, stdin) == 1) {
    printf("%d", ch[0]);
  }
}

int main(void) //int argc, char *argv[])
{
  //num_comp();
  num_decomp();
  fflush(stdout);
  return 0;
}

