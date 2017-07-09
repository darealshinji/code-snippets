/**
 * Compile with
 *   gcc -Wall -O3 -D_LARGEFILE64_SOURCE=1 -DHAVE_HIDDEN -Izlib -c -o crc32.o zlib/crc32.c
 *   gcc -Wall -O3 -Izlib crc32_check.c -s -o crc32_check crc32.o
 * or
 *   gcc -Wall -O3 crc32_check.c -s -o crc32_check -lz
 */

#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <zlib.h>

char *self;

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
  char error[256];
  snprintf(error, 255, "%s: %s", self, ch);
  perror(error);
}

long get_crc32(char *file)
{
  FILE *fp;
  uLong crc;
  char buf[4096];
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
  crc = crc32(0L, Z_NULL, 0);

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
      char crc_upper[9];
      char crc_lower[9];
      memset(crc_upper, '\0', 9);
      memset(crc_lower, '\0', 9);
      sprintf(crc_upper, "%.8lX", crc);
      sprintf(crc_lower, "%.8lx", crc);

      /* check if the filename contains the checksum */
      const char *match = "     ";
      if (file != NULL && (strstr(basename(file), crc_upper) != NULL ||
                           strstr(basename(file), crc_lower) != NULL))
      {
        match = "MATCH";
      }
      printf("%s %s %s\n", match, crc_upper, file);
    }
  }
}

int main(int argc, char *argv[])
{
  self = argv[0];

  if (argc == 1 || (argc == 2 && strcmp(argv[1], "--help") == 0))
  {
    printf("usage:\n");
    printf("  %s files\n", self);
    printf("  %s -\n", self);
    return 1;
  }
  else if (argc == 2 && strcmp(argv[1], "-") == 0)
  {
    /* read from stdin */
    crc_check(NULL);
  }
  else
  {
    /* process list of input */
    for (int i = 1; i < argc; ++i)
    {
      crc_check(argv[i]);
    }
  }

  return 0;
}

