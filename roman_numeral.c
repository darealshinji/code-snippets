#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN_BUF    17  /* MMMMDCCCLXXXVIII (= 4888) + null byte */
#define REVC       "\xE2\x86\x83" /* Ↄ */

#define APOS_TYPE  1

#if APOS_TYPE == 1
# define MIN_BUF_APOS  245
# define D500          "I" REVC " "
# define D5K           "I" REVC REVC " "
# define D50K          "I" REVC REVC REVC " "
# define D500K         "I" REVC REVC REVC REVC " "
# define M1K           "CI" REVC " "
# define M10K          "CCI" REVC REVC " "
# define M100K         "CCCI" REVC REVC REVC " "
# define M1MIO         "CCCCI" REVC REVC REVC REVC " "
#elif APOS_TYPE == 2
# define MIN_BUF_APOS  158
# define D500          "D"
# define D5K           "\xE2\x86\x81 "  /* ↁ */
# define D50K          "\xE2\x86\x87 "  /* ↇ */
# define D500K         "I" REVC REVC REVC REVC " "
# define M1K           "\xE2\x86\x80 "  /* ↀ */
# define M10K          "\xE2\x86\x82 "  /* ↂ */
# define M100K         "\xE2\x86\x88 "  /* ↈ */
# define M1MIO         "CCCCI" REVC REVC REVC REVC " "
#else
# error "APOS_TYPE"
#endif

/**
 * Converts a number between 1 and 4999 into a Roman numeral string (I, II, III, IV, etc.).
 * It returns a pointer to a given buffer, or to a new allocated string if buf was set to NULL.
 *
 * buf_size is ignored if buf was set to NULL, otherwise it must be at least 16.
 * On error it returns NULL and sets errno to 1 if the number was not between 1 and 4999 or
 * to EOVERFLOW if buf_size was lower than 16.
 */
char *roman_numeral(int n, char *buf, size_t buf_size)
{
  const char *str = NULL;

  if (n < 1 || n >= 5000) {
    errno = 1;  /* EPERM */
    return NULL;
  } else if (buf && buf_size < MIN_BUF) {
    errno = EOVERFLOW;
    return NULL;
  }

  if (buf == NULL) {
    buf = malloc(MIN_BUF);
  }

  buf[0] = '\0';

  if (n >= 1000) {
    switch (n / 1000) {
      case 1: str = "M";    break;
      case 2: str = "MM";   break;
      case 3: str = "MMM";  break;
      case 4: str = "MMMM"; break;
    }

    if (str) {
      strcat(buf, str);
      str = NULL;
    }
    n %= 1000;
  }

  if (n >= 100) {
    switch (n / 100) {
      case 9: str = "CM";   break;
      case 8: str = "DCCC"; break;
      case 7: str = "DCC";  break;
      case 6: str = "DC";   break;
      case 5: str = "D";    break;
      case 4: str = "CD";   break;
      case 3: str = "CCC";  break;
      case 2: str = "CC";   break;
      case 1: str = "C";    break;
    }

    if (str) {
      strcat(buf, str);
      str = NULL;
    }
    n %= 100;
  }

  if (n >= 10) {
    switch (n / 10) {
      case 9: str = "XC";   break;
      case 8: str = "LXXX"; break;
      case 7: str = "LXX";  break;
      case 6: str = "LX";   break;
      case 5: str = "L";    break;
      case 4: str = "XL";   break;
      case 3: str = "XXX";  break;
      case 2: str = "XX";   break;
      case 1: str = "X";    break;
    }

    if (str) {
      strcat(buf, str);
      str = NULL;
    }
    n %= 10;
  }

  switch (n) {
    case 0:
      return buf;
      break;
    case 1: str = "I";    break;
    case 2: str = "II";   break;
    case 3: str = "III";  break;
    case 4: str = "IV";   break;
    case 5: str = "V";    break;
    case 6: str = "VI";   break;
    case 7: str = "VII";  break;
    case 8: str = "VIII"; break;
    case 9: str = "IX";   break;
  }

  if (str) {
    strcat(buf, str);
  }

  return buf;
}

char *roman_numeral_apostrophus(int n, char *buf, size_t buf_size)
{
  const char *str = NULL;

  if (n < 1 || n >= 5000000) {
    errno = 1;  /* EPERM */
    return NULL;
  } else if (buf && buf_size < MIN_BUF_APOS) {
    errno = EOVERFLOW;
    return NULL;
  }

  if (buf == NULL) {
    buf = malloc(MIN_BUF_APOS);
  }

  buf[0] = '\0';

  /* 1.000.000 */
  if (n >= 1000000) {
    switch (n / 1000000) {
      case 1: str = M1MIO; break;
      case 2: str = M1MIO M1MIO; break;
      case 3: str = M1MIO M1MIO M1MIO; break;
      case 4: str = M1MIO M1MIO M1MIO M1MIO; break;
    }

    if (str) {
      strcat(buf, str);
      str = NULL;
    }
    n %= 1000000;
  }

  /* 100.000 */
  if (n >= 100000) {
    switch (n / 100000) {
      case 1: str = M100K; break;
      case 2: str = M100K M100K; break;
      case 3: str = M100K M100K M100K; break;
      case 4: str = M100K M100K M100K M100K; break;
      case 5: str = D500K; break;
      case 6: str = D500K M100K; break;
      case 7: str = D500K M100K M100K; break;
      case 8: str = D500K M100K M100K M100K; break;
      case 9: str = D500K M100K M100K M100K M100K; break;
    }

    if (str) {
      strcat(buf, str);
      str = NULL;
    }
    n %= 100000;
  }

  /* 10.000 */
  if (n >= 10000) {
    switch (n / 10000) {
      case 1: str = M10K; break;
      case 2: str = M10K M10K; break;
      case 3: str = M10K M10K M10K; break;
      case 4: str = M10K M10K M10K M10K; break;
      case 5: str = D50K; break;
      case 6: str = D50K M10K; break;
      case 7: str = D50K M10K M10K; break;
      case 8: str = D50K M10K M10K M10K; break;
      case 9: str = D50K M10K M10K M10K M10K; break;
    }

    if (str) {
      strcat(buf, str);
      str = NULL;
    }
    n %= 10000;
  }

  if (n >= 1000) {
    switch (n / 1000) {
      case 1: str = M1K; break;
      case 2: str = M1K M1K; break;
      case 3: str = M1K M1K M1K; break;
      case 4: str = M1K M1K M1K M1K; break;
      case 5: str = D5K; break;
      case 6: str = D5K M1K; break;
      case 7: str = D5K M1K M1K; break;
      case 8: str = D5K M1K M1K M1K; break;
      case 9: str = D5K M1K M1K M1K M1K; break;
    }

    if (str) {
      strcat(buf, str);
      str = NULL;
    }
    n %= 1000;
  }

  if (n >= 100) {
    switch (n / 100) {
      case 9: str = D500 "CCCC"; break;
      case 8: str = D500 "CCC";  break;
      case 7: str = D500 "CC";   break;
      case 6: str = D500 "C";    break;
      case 5: str = D500;        break;
      case 4: str = "CCCC";      break;
      case 3: str = "CCC";       break;
      case 2: str = "CC";        break;
      case 1: str = "C";         break;
    }

    if (str) {
      strcat(buf, str);
      str = NULL;
    }
    n %= 100;
  }

  if (n >= 10) {
    switch (n / 10) {
      case 9: str = "LXXXX"; break;
      case 8: str = "LXXX";  break;
      case 7: str = "LXX";   break;
      case 6: str = "LX";    break;
      case 5: str = "L";     break;
      case 4: str = "XXXX";  break;
      case 3: str = "XXX";   break;
      case 2: str = "XX";    break;
      case 1: str = "X";     break;
    }

    if (str) {
      strcat(buf, str);
      str = NULL;
    }
    n %= 10;
  }

  switch (n) {
    case 0:
      return buf;
      break;
    case 1: str = "I";     break;
    case 2: str = "II";    break;
    case 3: str = "III";   break;
    case 4: str = "IIII";  break;
    case 5: str = "V";     break;
    case 6: str = "VI";    break;
    case 7: str = "VII";   break;
    case 8: str = "VIII";  break;
    case 9: str = "VIIII"; break;
  }

  if (str) {
    strcat(buf, str);
  }

  return buf;
}

int multiplication_example(void)
{
  char buf[MIN_BUF];
  size_t sz = sizeof(buf);
  int n, m;

  n = 1234567;
  //n = 4321;
  //n = 12345;

  if (n < 5000) {
    if (roman_numeral(n, buf, sz)) {
      printf("%d = %s\n", n, buf);
      return 0;
    }
    goto failure;
  }
  else if (n >= 5000000) {
    printf("error: n >= 5000000\n");
    return 1;
  }

  if (!roman_numeral(n/1000, buf, sz)) {
    goto failure;
  }
  printf("%s*M", buf);

  if ((m = n % 1000) > 0) {
    if (!roman_numeral(m, buf, sz)) {
      goto failure;
    }
    printf(" %s", buf);
  }

  printf("\n");
  return 0;

failure:
  perror("roman_numeral()");
  return 1;
}

int main(void)
{
  //return multiplication_example();

  char *p = roman_numeral_apostrophus(4999999, NULL, 0);

  if (!p) {
    perror("roman_numeral_apostrophus()");
    return 1;
  }

  printf("%s (strlen == %ld)\n", p, strlen(p));
  free(p);
  return 0;
}

