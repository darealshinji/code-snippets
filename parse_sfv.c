// TODO: do some actual CRC checks

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct sfv_file_list
{
  char entry[1024];      /* filename/basename */
  char path[PATH_MAX];   /* full resolved path */
  char crcex[9];         /* expected CRC32 */
  char crc[9];           /* actual CRC32 */
};

size_t parse_sfv(const char *file, struct sfv_file_list *list)
{
  FILE *fp;
  char buf[sizeof(list[0].entry)];
  char y, z;
  size_t len, n = 0, line = 0, count = 0;

  fp = fopen(file, "r");

  if (fp == NULL)
  {
    perror("fopen()");
    return -1;
  }

  if (fseek(fp, 0, SEEK_END) == -1)
  {
    perror("fseek()");
    fclose(fp);
    return -1;
  }

  rewind(fp);

  while (fgets(buf, sizeof(buf), fp) != NULL)
  {
    len = strlen(buf);
    line++;

    if (len > 10 && buf[0] != ';')
    {
      y = buf[len - 2];
      z = buf[len - 1];

      /* remove newline characters:
       * \r\n = Windows; \n = OSX/Unix; \r = Classic Mac */
      if (y == '\r' && z == '\n')
      {
        buf[len - 2] = '\0';
        buf[len - 1] = '\0';
      }
      else if (z == '\n' || z == '\r')
      {
        buf[len - 1] = '\0';
      }
      len = strlen(buf);

      if (buf[len - 9] != ' ')
      {
        fprintf(stderr, "%s: error at line %ld, character %ld\n", file, line, len - 8);
        fclose(fp);
        return -1;
      }

      count = n + 1;
      snprintf(list[n].entry, len - 8, "%s", buf);
      snprintf(list[n].crcex, 9, "%s", buf + len - 8);
      list[n].crc[0] = '\0';

      if (realpath(list[n].entry, list[n].path) == NULL)
      {
        list[n].path[0] = '\0';
      }
      n++;
    }
  }

  fclose(fp);
  return count;
}

int main(void)
{
  struct sfv_file_list list[1000];
  const char *file = "test.sfv";
  char *path;
  size_t i, n;

  n = parse_sfv(file, list);

  if (n < 1)
  {
    return 1;
  }

  for (i = 0; i < n; ++i)
  {
    path = list[i].path;

    if (path[0] == '\0')
    {
      path = "<NOT FOUND>";
    }
    printf("[%s] %s -> %s\n", list[i].crcex, list[i].entry, path);
  }
  printf("%ld files\n", n);

  return 0;
}

