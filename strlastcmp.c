#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Writes string src in reversed order to the buffer pointed to by dest;
 * dest will be terminated by a null byte ('\0')
 */
static inline
void revstr(char *dest, const char *src)
{
  size_t i = 0;
  size_t j = strlen(src) - 1;

  for (; i <= j; ++i)
  {
    dest[i] = src[j - i];
  }
  dest[i] = '\0';
}

/* Compares the last n bytes of str1 and str2 */
int strlastcmp(const char *str1, const char *str2, size_t n)
{
  size_t size = strlen(str1);
  char *str1r = (char *)malloc(size);
  memset(str1r, '\0', size);
  revstr(str1r, str1);

  size = strlen(str2);
  char *str2r = (char *)malloc(size);
  memset(str2r, '\0', size);
  revstr(str2r, str2);

  int ret = strncmp(str1r, str2r, n);
  free(str1r);
  free(str2r);
  return ret;
}

int main()
{
  const char *str1 = "string";
  const char *str2 = "king";
  size_t n = 3;

  if (strlastcmp(str1, str2, n) == 0)
  {
    printf("yes\n");
  }
  else
  {
    printf("no\n");
  }

  return 0;
}

