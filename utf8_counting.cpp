#include <string>
#include <stdio.h>
#include <string.h>

// count utf8 characters in p
size_t utf8len(const char *p)
{
  size_t i = 0;
  for ( ; *p != 0; ++p) {
    i += ((*p & 0xc0) != 0x80);
  }
  return i;
}

// get the byte range from a utf8 character range, i.e. for
// use with std::string.substr()
//
// p: input string
// utf8_start: first utf8 character in range
// utf8_length: amount of utf8 characters in range
// byte_start: number of the first byte of the first utf8 character
// byte_length: byte length of utf8 character range
// return value: pointer to first byte of the first utf8 character
const char *utf8_range(const char *p, size_t utf8_start, size_t utf8_length, size_t &byte_start, size_t &byte_length)
{
  const char *r = p;
  size_t utf8 = 0;
  size_t utf8_range = 0;
  size_t i = 0, bytes = 0;
  byte_start = byte_length = 0;

  while (*p != 0) {
    switch (0xF0 & *p) {
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

    if (utf8 == utf8_start) {
      byte_start = i;
      r = p;
    }

    if (utf8 >= utf8_start) {
      utf8_range++;
      byte_length += bytes;
    }

    if (utf8_length > 0 && utf8_range == utf8_length) {
      break;
    }

    i += bytes;
    p += bytes;
    utf8++;
  }

  return r;
}

int main()
{
  const char *s = "M" "\xC3\xB6" "tley Cr" "\xC3\xBC" "e";  // Mötley Crüe

  size_t cStart = 7;
  size_t cLen = 3;
  size_t bStart = 0;
  size_t bLen = 0;

  //utf8_range(s, cStart, cLen, bStart, bLen);
  std::string s2 = utf8_range(s, cStart, cLen, bStart, bLen);

  printf("strlen() == %zd\n", strlen(s));
  printf("utf8len() == %zd\n", utf8len(s));
  printf("'%s' -> '%s'\n", s, s2.substr(0, bLen).c_str());
  printf("utf8[%zd,%zd]  bytes[%zd,%zd]\n", cStart, cLen, bStart, bLen);

  return 0;
}
