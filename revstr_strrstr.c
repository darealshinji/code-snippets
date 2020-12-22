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

/* Alternate version to directly write into an array.
 * Returns a pointer to the array.
 */
char *revstr2(char *dest, size_t szdest, const char *source);

/* Finds the LAST occurrence of the substring needle in the string haystack.
 * It returns a pointer to the beginning of the located substring, or NULL
 * if the substring is not found.
 */
char *strrstr(const char *haystack, const char *needle);

/* Like strrstr(), but it ignores the case of both arguments. */
char *strrcasestr(const char *haystack, const char *needle);



char *revstr(const char *s)
{
  char *r;
  size_t i, len;

  if (!s) {
    return NULL;
  }

  len = strlen(s);

  if (len == 0) {
    return NULL;
  } else if (len == 1) {
    return strdup(s);
  }

  r = malloc(len);
  --len;

  for (i = 0; i <= len; ++i) {
    r[i] = s[len - i];
  }
  r[i] = '\0';

  return r;
}

char *revstr_utf8(const char *s)
{
  char *r;
  size_t len, bytes;

  if (!s) {
    return NULL;
  }

  len = strlen(s);

  if (len == 0) {
    return NULL;
  } else if (len == 1) {
    return strdup(s);
  }

  r = malloc(len);
  r[len] = '\0';

  while (*s != 0) {
    switch (0xF0 & *s) {
      case 0xC0:
        bytes = 2;
        break;
      case 0xE0:
        bytes = 3;
        break;
      case 0xF0:
        bytes = 4;
        break;
      default:
        bytes = 1;
        break;
    }

    len -= bytes;
    strncpy(r+len, s, bytes);
    s += bytes;
  }

  return r;
}

char *revstr2(char *dest, size_t szdest, const char *source)
{
  size_t i, j, k;

  if (!source) {
    return NULL;
  }

  j = strlen(source);

  if (j == 0) {
    return NULL;
  }

  --j;
  k = szdest - 1;

  if (k < j) {
    j = k;
  }

  for (i = 0; i <= j; ++i) {
    dest[i] = source[j - i];
  }
  dest[i] = '\0';

  return dest;
}

char *revstr2_utf8(char *dest, size_t szdest, const char *source)
{
  size_t len, i, bytes;
  char *r;

  if (!source) {
    return NULL;
  }

  len = strlen(source);

  if (len == 0) {
    return NULL;
  }

  if (szdest < len) {
    len = szdest;
  }

  r = revstr_utf8(source);

  if (!r) {
    return NULL;
  }

  for (i=0, bytes=0; i < len; i+=bytes) {
    switch (0xF0 & r[i]) {
      case 0xC0:
        bytes = 2;
        break;
      case 0xE0:
        bytes = 3;
        break;
      case 0xF0:
        bytes = 4;
        break;
      default:
        bytes = 1;
        break;
    }

    if (i + bytes >= len) {
      r[i] = '\0';
      break;
    }
  }

  strcpy(dest, r);
  free(r);

  return dest;
}

char *strrstr(const char *haystack, const char *needle)
{
  char *a, *b, *c, *d;
  a = revstr(haystack);
  b = revstr(needle);
  c = strstr(a, b);
  d = c ? (char *)haystack + strlen(c) - strlen(needle) : NULL;
  free(a);
  free(b);
  return d;
}

char *strrcasestr(const char *haystack, const char *needle)
{
  char *a, *b, *c, *d;
  a = revstr(haystack);
  b = revstr(needle);
  c = strcasestr(a, b);
  d = c ? (char *)haystack + strlen(c) - strlen(needle) : NULL;
  free(a);
  free(b);
  return d;
}

/* tests */
int main(void)
{
  const char *str = "Hello world! Why does the world never response? WHY?";
  char *p1, *p2;
  char arr[8] = {0};

  printf("\nstr = \"%s\"\nsizeof(arr) = %ld\n\n", str, sizeof(arr));

  p1 = revstr(str);
  printf("revstr(str)\n>>> \"%s\"\n\n", p1);

  revstr2(arr, sizeof(arr), str);
  printf("revstr2(arr, sizeof(arr), str)\n>>> \"%s\"\n\n", arr);

  printf("strstr(str, \"world\")\n>>> \"%s\"\n\n", strstr(str, "world"));
  printf("strrstr(str, \"world\")\n>>> \"%s\"\n\n", strrstr(str, "world"));
  printf("strcasestr(str, \"wHy\")\n>>> \"%s\"\n\n", strcasestr(str, "wHy"));
  printf("strrcasestr(str, \"wHy\")\n>>> \"%s\"\n\n", strrcasestr(str, "wHy"));

  p2 = revstr_utf8("abcÄöüß");
  printf("revstr_utf8(\"abcÄöüß\")\n>>> \"%s\"\n\n", p2);

  revstr2_utf8(arr, sizeof(arr), "abcÄöüß");
  printf("revstr2_utf8(arr, sizeof(arr), \"abcÄöüß\")\n>>> \"%s\"\n\n", arr);

  free(p1);
  free(p2);

  return 0;
}

