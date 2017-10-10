#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Returns a pointer to a new string which is a duplicate of the string s
 * but with bytes in reversed order ("abcdef" becomes "fedcba").
 * A terminating null byte ('\0') is added. The string must be freed later.
 */
char *revstr(const char *s)
{
  char *r = NULL;
  size_t i, j;

  if (s != NULL)
  {
    j = strlen(s);
    r = malloc(j);
    --j;

    for (i = 0; i <= j; ++i)
    {
      r[i] = s[j - i];
    }
    r[i] = '\0';
  }

  return r;
}

int main(void)
{
  const char *str = "abcdef";
  char *rev = NULL;

  rev = revstr(str);
  printf("%s -> %s\n", str, rev);

  free(rev);
  return 0;
}

