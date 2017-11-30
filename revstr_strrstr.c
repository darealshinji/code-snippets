#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Returns a pointer to a new string which is a duplicate of the string s
 * but with bytes in reversed order ("abcdef" becomes "fedcba").
 * A terminating null byte ('\0') is added. The string must be freed later.
 */
char *revstr(const char *s);

/* Finds the LAST occurrence of the substring needle in the string haystack.
 * It returns a pointer to the beginning of the located substring, or NULL
 * if the substring is not found.
 */
char *strrstr(const char *haystack, const char *needle);

/* Like strrstr(), but it ignores the case of both arguments. */
char *strrcasestr(const char *haystack, const char *needle);



char *revstr(const char *s)
{
  char *r = NULL;
  size_t i, j;

  if (s)
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

/* alternate version to directly write into an array */
/*
void revstr(char *dest, const char *source)
{
  size_t i, j;

  if (source)
  {
    j = strlen(source) - 1;

    for (i = 0; i <= j; ++i)
    {
      dest[i] = source[j - i];
    }
    dest[i] = '\0';
  }
}
*/

char *_strrstr(const char *hs, const char *ndl, char ic)
{
  char *a, *b, *c, *d;
  a = revstr(hs);
  b = revstr(ndl);
  c = ic ? strcasestr(a, b) : strstr(a, b);
  d = c ? (char *)hs + strlen(c) - strlen(ndl) : NULL;
  free(a);
  free(b);
  return d;
}

char *strrstr(const char *haystack, const char *needle)
{
  return _strrstr(haystack, needle, 0);
}

char *strrcasestr(const char *haystack, const char *needle)
{
  return _strrstr(haystack, needle, 1);
}

/* tests */
int main(void)
{
  const char *str = "Hello world! Why does the world never response? WHY?";
  char *rev = NULL;

  printf("\nstr = \"%s\"\n\n", str);
  printf("revstr(str)\n>>> %s\n\n", rev = revstr(str));
  printf("strstr(str, \"world\")\n>>> \"%s\"\n\n", strstr(str, "world"));
  printf("strrstr(str, \"world\")\n>>> \"%s\"\n\n", strrstr(str, "world"));
  printf("strcasestr(str, \"wHy\")\n>>> \"%s\"\n\n", strcasestr(str, "wHy"));
  printf("strrcasestr(str, \"wHy\")\n>>> \"%s\"\n\n", strrcasestr(str, "wHy"));

  free(rev);
  return 0;
}

