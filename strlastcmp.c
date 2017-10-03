#include <stdio.h>
#include <string.h>

size_t strlastcmp(const char *s1, const char *s2);
int str_ends_on(const char *s, const char *suf);

/* Compares the last bytes of s1 and s2 and returns the number of equal bytes.
 * The terminating null byte ('\0') is ignored.
 * It returns -1 if the length of s1 and/or s2 is 0.
 */
size_t strlastcmp(const char *s1, const char *s2)
{
  size_t len1 = strlen(s1);
  size_t len2 = strlen(s2);
  size_t n = 0;

  if (len1 == 0 || len2 == 0)
  {
    return -1;
  }

  while (len1 > 0 && len2 > 0)
  {
    if (s1[len1 - 1] != s2[len2 - 1])
    {
      break;
    }
    ++n;
    --len1;
    --len2;
  }

  return n;
}

/* Returns 1 if string s ends on string suf, otherwise 0 is returned. */
int str_ends_on(const char *s, const char *suf)
{
  if (strlastcmp(s, suf) == strlen(suf))
  {
    return 1;
  }
  return 0;
}

int main(void)
{
  const char *str1 = "Budapest",
    *str2 = "Bukarest",
    *str3 = "abcdef",
    *str4 = "SONIC.exe",
    *suf = ".exe",
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
  if (str_ends_on(str4, suf))
  {
    yesno = "ends";
  }
  printf("Test 3\n");
  printf("string: %s\n", str4);
  printf("suffix: %s\n", suf);
  printf("`%s' %s on `%s'\n", str4, yesno, suf);

  return 0;
}

