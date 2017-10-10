/**
 * A small CRC32 checksum tool using zlib's or stb's crc32() function.
 *
 * Compile against zlib:
 *   gcc -Wall -O3 crc32_check.c -s -o crc32_check -lz
 *
 * Compile with built-in stb crc function (slower):
 *   gcc -Wall -O3 -DSTB_DEFINE crc32_check.c -s -o crc32_check
 *
 * Compile with built-in zlib sources:
 *   zlibDir="path/to/zlib/sources"
 *   gcc -Wall -O3 -march=native -DUSE_ZLIB -D_LARGEFILE64_SOURCE=1 -DHAVE_HIDDEN -I"$zlibDir" -c -o crc32.o "$zlibDir/crc32.c"
 *   gcc -Wall -O3 -march=native -I"$zlibDir" crc32_check.c -s -o crc32_check crc32.o
 */

#include <stdio.h>
#include <string.h>
#include <libgen.h>

#ifdef STB_DEFINE
typedef unsigned int uInt;
typedef unsigned long uLong;
typedef unsigned char Bytef;
typedef uInt crc32_t;
#else
#include <zlib.h>
typedef uLong crc32_t;
#endif

char *perror_wrapper_self = NULL;

static inline
void progress(uInt n)
{
  char *l = "====================================================================================================";
  char *r = "----------------------------------------------------------------------------------------------------";
  char *s = "                                                                                                    ";

  if (n < 100)
  {
    /*  0% |>--..---|
     *  1% |=>-..---|
     * 98% |===..=>-|
     * 99% |===..==>| */
    fprintf(stderr, "|%s>%s|\r", l+100-n, r+n+1);
  }
  else
  {
    /* clear output with spaces */
    fprintf(stderr, " %s \r", s);
  }
}

static inline
void perror_wrapper(char *ch)
{
  if (perror_wrapper_self)
  {
    char error[4352] = {0};
    snprintf(error, 4351, "%s: %s", perror_wrapper_self, ch);
    perror(error);
  }
  else
  {
    perror(ch);
  }
}

/**
 * https://github.com/nothings/stb
 * from stb_crc32_block()
 */
#ifdef STB_DEFINE
static inline
uInt crc32(uInt crc, const Bytef *buffer, uInt len)
{
  static uInt crc_table[256];
  uInt i, j, s;
  crc = ~crc;

  if (crc_table[1] == 0)
  {
    for (i = 0; i < 256; i++)
    {
      for (s = i, j = 0; j < 8; ++j)
      {
        s = (s >> 1) ^ (s & 1 ? 0xedb88320 : 0);
      }
      crc_table[i] = s;
    }
  }

  for (i = 0; i < len; ++i)
  {
    crc = (crc >> 8) ^ crc_table[buffer[i] ^ (crc & 0xff)];
  }
  return ~crc;
}
#endif

long get_crc32(char *file)
{
  FILE *fp;
  crc32_t crc;
  Bytef buf[262144]; /* 256k */
  long fileSize = 0L;
  long byteCount = 0L;
  uInt completed = 0;

  if (file == NULL)
  {
    /* read from stdin */
    fp = stdin;
  }
  else
  {
    /* open file for reading */
    fp = fopen(file, "r");

    if (fp == NULL)
    {
      perror_wrapper(file);
      return -1;
    }

    /* seek to end of file */
    if (fseek(fp, 0, SEEK_END) == -1)
    {
      perror_wrapper(file);
      fclose(fp);
      return -1;
    }

    /* get filesize */
    fileSize = ftell(fp);

    /* set position indicator back
     * to the beginning */
    rewind(fp);
  }

  /* initialize crc */
  crc = crc32(0, NULL, 0);

  /* loop until we reach the end */
  while (feof(fp) == 0)
  {
    /* read data into buffer */
    size_t items = fread(buf, sizeof(*buf), sizeof(buf)/sizeof(*buf), fp);

    if (ferror(fp) != 0)
    {
      perror_wrapper(file);
      if (file != NULL)
      {
        fclose(fp);
      }
      return -1;
    }

    if (file != NULL)
    {
      /* calculate and print progress */
      byteCount += (long)(items * sizeof(*buf));
      uInt n = (uInt)((float)byteCount/(float)fileSize*100.0);

      /* print only if the counter has changed */
      if (n > completed)
      {
        completed = n;
        progress(n);
      }
    }

    /* calculate checksum and append to crc */
    crc = crc32(crc, (const Bytef *)buf, (uInt)(items * sizeof(*buf)));
  }

  if (file != NULL)
  {
    fclose(fp);
  }

  return (long)crc;
}

void crc_check(char *file)
{
  long crc = get_crc32(file);

  if (crc != -1)
  {
    if (file == NULL)
    {
      /* input was stdin, just print the checksum */
      printf("%.8lX\n", crc);
    }
    else
    {
      char crc_upper[9] = {0};
      char crc_lower[9] = {0};
      snprintf(crc_upper, 9, "%.8lX", crc);
      snprintf(crc_lower, 9, "%.8lx", crc);

      /* check if the filename contains the checksum */
      char *match;
      if (strstr(basename(file), crc_upper) != NULL || strstr(basename(file), crc_lower) != NULL)
      {
        match = "OK";
      }
      else
      {
        match = "--";
      }
      printf("%s %s %s\n", crc_upper, match, file);
    }
  }
}

int main(int argc, char *argv[])
{
  perror_wrapper_self = basename(argv[0]);

  if (argc == 1)
  {
    /* read from stdin */
    crc_check(NULL);
  }
  else
  {
    /* process list of input files */
    for (int i = 1; i < argc; ++i)
    {
      crc_check(argv[i]);
    }
  }

  return 0;
}

