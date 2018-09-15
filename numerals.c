/* convert an unsigned long value into a numeral string of a given system */

#include <stdio.h>
#include <string.h>

enum {
  NUM_DEVANAGARI,
  NUM_GUJARATI,
  NUM_GURMUKHI,
  NUM_TIBETAN,
  NUM_BENGALI,
  NUM_KANNADA,
  NUM_ODIA,
  NUM_MALAYALAM,
  NUM_TAMIL,
  NUM_TELUGU,
  NUM_KHMER,
  NUM_THAI,
  NUM_LAO,
  NUM_BURMESE,
  NUM_ARABIC,
  NUM_PERSIAN
};

int num_to_string(char *str, size_t size, int system, unsigned long n)
{
  char z[] = { '\xE0','\xA5','\xA6', 0 };  /* zero in Devanagari */
  int i, j, len = 3;

  switch (system) {
    case NUM_DEVANAGARI:  break;
    case NUM_GUJARATI:    z[1] = '\xAB'; break;
    case NUM_GURMUKHI:    z[1] = '\xA9'; break;
    case NUM_TIBETAN:     z[1] = '\xBC'; z[2] = '\xA0'; break;
    case NUM_BENGALI:     z[1] = '\xA7'; break;
    case NUM_KANNADA:     z[1] = '\xB3'; break;
    case NUM_ODIA:        z[1] = '\xAD'; break;
    case NUM_MALAYALAM:   z[1] = '\xB5'; break;
    case NUM_TAMIL:       z[1] = '\xAF'; break;
    case NUM_TELUGU:      z[1] = '\xB1'; break;
    case NUM_KHMER:       z[0] = '\xE1'; z[1] = '\x9F'; z[2] = '\xA0'; break;
    case NUM_THAI:        z[1] = '\xB9'; z[2] = '\x90'; break;
    case NUM_LAO:         z[1] = '\xBB'; z[2] = '\x90'; break;
    case NUM_BURMESE:     z[0] = '\xE1'; z[1] = '\x81'; z[2] = '\x80'; break;
    case NUM_ARABIC:      z[0] = '\xD9'; z[2] = '\xA0'; len = 2; break;
    case NUM_PERSIAN:     z[0] = '\xDB'; z[2] = '\xB0'; len = 2; break;
    default:
      break;
  }

  /* only one digit */
  if (n < 10) {
    str[0] = z[0];
    if (len == 3) {
      str[1] = z[1];
      i = 1;
    } else {
      i = 0;
    }
    str[1+i] = z[2] + n;
    str[2+i] = '\0';
    return len;
  }

  /* insert last byte, reversing the order */
  for (i=0; i < (size - 1) && n != 0; i+=len, n/=10) {
    str[i] = z[2] + n%10;
  }

  /* return 0 length if buffer wasn't large enough */
  if (n > 0) {
    str[0] = '\0';
    return 0;
  }

  /* correct order, add missing first bytes */
  for (j=0, --i; i >= 1; j+=len, i-=len) {
    str[i] = str[j];
    str[j] = z[0];
    if (len == 3) {
      str[j+1] = z[1];
    }
  }
  str[j] = '\0';

  return j;
}

int main(void)
{
  char buf[64];
  int len = num_to_string(buf, sizeof(buf), NUM_TAMIL, 2015);
  printf("%s\n", buf);
  printf("%d bytes\n", len);
  return 0;
}

