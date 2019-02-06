/**
 * Simple code-generator to quote and escape text read from STDIN
 * so it can be used in C/C++, i.e. to initialize a char array.
 * The program will stop on the first occurance of the null byte ('\0').
 * Use the xxd command if you need to create a complete array from
 * binary data.
 */

#include <stdio.h>

#define MAX_LINE_LENGTH 80


int main(void)
{
  char c = getchar();
  char s[9] = {0};
  int i, n = 0;

  if (c == EOF || c == '\0') {
    return 1;
  }

  printf("  \"");
  i = 3;

  while (c != EOF && c != 0) {
    if (c >= ' ' && c <= '~') {
      /* printable ASCII chars */
      switch (c) {
        case '"': s[0]='\\'; s[1]='"'; s[2]=0; n=2; break;
        case '\\': s[0]='\\'; s[1]='\\'; s[2]=0; n=2; break;
        default:
          s[0]=c; s[1]=0;
          n = 1;
          break;
      }
    } else if (c >= '\a' && c <= '\r') {
      /* control characters with escape sequence (except '\e') */
      switch (c) {
        case '\n': s[0]='\\'; s[1]='n'; s[2]=0; n=2; break;
        case '\r': s[0]='\\'; s[1]='r'; s[2]=0; n=2; break;
        case '\t': s[0]='\\'; s[1]='t'; s[2]=0; n=2; break;
        case '\v': s[0]='\\'; s[1]='v'; s[2]=0; n=2; break;
        case '\f': s[0]='\\'; s[1]='f'; s[2]=0; n=2; break;
        case '\a': s[0]='\\'; s[1]='a'; s[2]=0; n=2; break;
        case '\b': s[0]='\\'; s[1]='b'; s[2]=0; n=2; break;
      }
    } else {
      snprintf(s, 9, "\"\"\\x%02X\"\"", (unsigned char)c);
      n = 8;
    }

    if ((i + n) >= MAX_LINE_LENGTH) {
      printf("\"\n  \"%s", s);
      i = 3 + n;
    } else {
      printf("%s", s);
      i += n;
    }

    c = getchar();
  }
  printf("\"\n");

  return 0;
}

