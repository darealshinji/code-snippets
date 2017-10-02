#include <stdio.h>
#include <string.h>

/* Returns a pointer to a new string which is a duplicate of the string s
 * but with bytes in reversed order ("abcdef" becomes "fedcba").
 * A terminating null byte ('\0') is added.
 */
char *revstr(const char *s)
{
  char *r = strdup(s);
  size_t i = 0;
  size_t j = strlen(s) - 1;

  for (; i <= j; ++i)
  {
    r[i] = s[j - i];
  }
  r[i] = '\0';

  return r;
}

int main(void)
{
  const char *str = "abcdef";
  char *rev = revstr(str);
  printf("%s -> %s\n", str, rev);
  return 0;
}

