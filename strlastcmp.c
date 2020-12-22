#include <ctype.h>
#include <stdio.h>
#include <string.h>

size_t strlastcmp(const char *s1, const char *s2);
size_t strlastcasecmp(const char *s1, const char *s2);
int str_ends_on(const char *s, const char *suf);
int strcase_ends_on(const char *s, const char *suf);

/* Compares the last bytes of s1 and s2 and returns the number of equal bytes.
 * The terminating null byte ('\0') is ignored.
 * It returns -1 if the length of s1 and/or s2 is 0.
 */
size_t strlastcmp(const char *s1, const char *s2)
{
  size_t len1 = strlen(s1);
  size_t len2 = strlen(s2);
  size_t n = 0;

  if (len1 == 0 || len2 == 0) {
    return -1;
  }

  while (len1 > 0 && len2 > 0) {
    if (s1[len1 - 1] != s2[len2 - 1]) {
      break;
    }
    ++n; --len1; --len2;
  }
  return n;
}

/* like strlastcmp() but ignoring case */
size_t strlastcasecmp(const char *s1, const char *s2)
{
  size_t len1 = strlen(s1);
  size_t len2 = strlen(s2);
  size_t n = 0;

  if (len1 == 0 || len2 == 0) {
    return -1;
  }

  while (len1 > 0 && len2 > 0) {
    if (tolower(s1[len1 - 1]) != tolower(s2[len2 - 1])) {
      break;
    }
    ++n; --len1; --len2;
  }
  return n;
}

/* returns 0 if s ends on suf */
int str_ends_on(const char *s, const char *suf)
{
  size_t len1 = strlen(s);
  size_t len2 = strlen(suf);

  if (len1 == 0 || len2 == 0 || len1 < len2) {
    return -1;
  }

  return strcmp(s + (len1 - len2), suf);
}

/* like str_ends_on() but ignoring case */
int strcase_ends_on(const char *s, const char *suf)
{
  size_t len1 = strlen(s);
  size_t len2 = strlen(suf);

  if (len1 == 0 || len2 == 0 || len1 < len2) {
    return -1;
  }

  return strcasecmp(s + (len1 - len2), suf);
}

int main(void)
{
  const char *str1 = "Budapest",
    *str2 = "Bukarest",
    *str3 = "abcdef",
    *str4 = "SONIC.exe",
    *suf1 = ".exe",
    *suf2 = ".EXE",
    *yesno = "does NOT end";
  size_t n;

  /* Test 1 */
  n = strlastcmp(str1, str2);
  printf("Test 1\n");
  printf("string 1: %s\n", str1);
  printf("string 2: %s\n", str2);
  printf("last common chars: %ld\n\n", n);

  /* Test 2 */
  n = strlastcmp(str3, str3);
  printf("Test 2\n");
  printf("string 1: %s\n", str3);
  printf("string 2: %s\n", str3);
  printf("last common chars: %ld\n\n", n);

  /* Test 3 */
  if (str_ends_on(str4, suf1) == 0) {
    yesno = "ends";
  }
  printf("Test 3\n");
  printf("string: %s\n", str4);
  printf("suffix: %s\n", suf1);
  printf("`%s' %s on `%s'\n\n", str4, yesno, suf1);

  /* Test 4 */
  if (strcase_ends_on(str4, suf2) == 0) {
    yesno = "ends";
  } else {
    yesno = "does NOT end";
  }
  printf("Test 4\n");
  printf("string: %s\n", str4);
  printf("suffix: %s\n", suf2);
  printf("`%s' %s on `%s' (ignoring case)\n\n", str4, yesno, suf2);

  return 0;
}

