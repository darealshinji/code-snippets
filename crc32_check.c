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

#ifdef _MSC_VER
#include <Windows.h>
#else
#include <libgen.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef STB_DEFINE
typedef unsigned char Bytef;
typedef unsigned int crc32_t;
#define TOHEX "%.8X"
#else
#include <zlib.h>
typedef uLong crc32_t;
#define TOHEX "%.8lX"
#endif

#define SHOW_PROGRESS
#define BUFSIZE 262144  /* 256k */

char *self = NULL;
int get_crc32_error = 0;

#ifdef _MSC_VER
/* not exaclty like the real POSIX function, but close enough */
char *basename(char *path)
{
  char fname[_MAX_FNAME];
  char ext[_MAX_EXT];

  if (_splitpath_s(path, NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT) != 0)
  {
    return NULL;
  }
  _snprintf_s(path, strlen(path), _TRUNCATE, "%s%s", fname, ext);

  return path;
}
#endif

#ifdef SHOW_PROGRESS
void progress(unsigned int n)
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
#endif

void perror_wrapper(char *ch)
{
  if (self)
  {
    char error[4352] = {0};
    snprintf(error, 4351, "%s: %s", self, ch);
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
crc32_t crc32(crc32_t crc, Bytef *buffer, size_t len)
{
  crc32_t crc_table[256], i, j, s;
  crc = ~crc;

  for (i = 0; i < 256; i++)
  {
    for (s = i, j = 0; j < 8; ++j)
    {
      s = (s >> 1) ^ (s & 1 ? 0xEDB88320 : 0);
    }
    crc_table[i] = s;
  }

  for (i = 0; i < len; ++i)
  {
    crc = (crc >> 8) ^ crc_table[buffer[i] ^ (crc & 0xFF)];
  }

  return ~crc;
}
#endif

crc32_t get_crc32(char *file)
{
  FILE *fp;
  crc32_t crc;
  Bytef buf[BUFSIZE];
  size_t items;
#ifdef SHOW_PROGRESS
  long fileSize = 0L;
  size_t byteCount = 0;
  unsigned int completed = 0;
#endif

  get_crc32_error = 0;

  if (file == NULL)
  {
    /* read from stdin */
    fp = stdin;
  }
  else
  {
    /* open file for reading;
     * do NOT omit the "b" on Windows! */
    fp = fopen(file, "rb");

    if (fp == NULL)
    {
      perror_wrapper(file);
      get_crc32_error = 1;
      return 0;
    }

    /* seek to end of file */
    if (fseek(fp, 0, SEEK_END) == -1)
    {
      perror_wrapper(file);
      fclose(fp);
      get_crc32_error = 1;
      return 0;
    }

#ifdef SHOW_PROGRESS
    /* get filesize */
    fileSize = ftell(fp);
#endif

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
    items = fread(buf, 1, BUFSIZE, fp);

    if (ferror(fp) != 0)
    {
      perror_wrapper(file);
      if (file != NULL)
      {
        fclose(fp);
      }
      get_crc32_error = 1;
      return 0;
    }

#ifdef SHOW_PROGRESS
    if (file != NULL)
    {
      /* calculate and print progress */
      byteCount += items;
      unsigned int n = (unsigned int)((float)byteCount/(float)fileSize*100.0);

      /* print only if the counter has changed */
      if (n > completed)
      {
        completed = n;
        progress(n);
      }
    }
#endif

    /* calculate checksum and append to crc */
    crc = crc32(crc, buf, items);
  }

  if (file != NULL)
  {
    fclose(fp);
  }

  return crc;
}

void crc_check(char *file)
{
  crc32_t crc;
  char crc_upper[9] = {0};
  char crc_lower[9] = {0};
  char *fname = NULL;
  const char *match = "--";

  crc = get_crc32(file);

  if (get_crc32_error == 0)
  {
    if (file == NULL)
    {
      /* input was stdin, just print the checksum */
      printf(TOHEX "\n", crc);
    }
    else
    {
      fname = basename(strdup(file));
      snprintf(crc_upper, 9, TOHEX, crc);
      snprintf(crc_lower, 9, TOHEX, crc);

      /* check if the filename contains the checksum */
      if (strstr(fname, crc_upper) || strstr(fname, crc_lower))
      {
        match = "OK";
      }
      printf("%s %s %s\n", crc_upper, match, file);
      free(fname);
    }
  }
}

int main(int argc, char *argv[])
{
  self = basename(argv[0]);

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

