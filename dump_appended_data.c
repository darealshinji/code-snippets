#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/**
 * gcc -Wall -O2 dump_appended_data.c -s
 * str1="P_L_A_C_E_H_O_L_D_E_R"
 * str2="000000000000000006392"  # file size
 * sed -i "s|$str1|$str2|" a.out
 * cat a.out file.bin > dump_file.run
 */

int main(void)
{
  char self[PATH_MAX + 1] = {0};
  char out[PATH_MAX + 6] = {0};
  char buf[262144]; /* 256k */
  long length;
  ssize_t size, items;
  FILE *fd, *fdOut;

  length = atol("P_L_A_C_E_H_O_L_D_E_R");

  if (length == 0)
  {
    fprintf(stderr, "error: size of executable header not set\n");
    return 1;
  }

  size = readlink("/proc/self/exe", self, PATH_MAX);

  if (size == -1)
  {
    fprintf(stderr, "error: readlink(): cannot read `/proc/self/exe'\n");
    return 1;
  }

  sprintf(out, "%s_dump", self);

  fd = fopen(self, "r");
  fdOut = fopen(out, "w");

  if (!fd || !fdOut)
  {
    fprintf(stderr, "error: fopen()\n");
    return 1;
  }

  if (fseek(fd, length, SEEK_SET) == -1)
  {
    fprintf(stderr, "error: fseek()\n");
    if (fd) { fclose(fd); }
    if (fdOut) { fclose(fdOut); }
    return 1;
  }

  while (feof(fd) == 0)
  {
    items = fread(buf, sizeof(*buf), sizeof(buf)/sizeof(*buf), fd);

    if (ferror(fd) != 0)
    {
      fprintf(stderr, "error: fread()\n");
      if (fd) { fclose(fd); }
      if (fdOut) { fclose(fdOut); }
      return 1;
    }

    fwrite(buf, 1, items, fdOut);

    if (ferror(fdOut) != 0)
    {
      fprintf(stderr, "error: fwrite()\n");
      if (fd) { fclose(fd); }
      if (fdOut) { fclose(fdOut); }
      return 1;
    }
  }

  if (fd) { fclose(fd); }
  if (fdOut) { fclose(fdOut); }

  /* make executable? */
  //chmod(out, 00755);

  return 0;
}

