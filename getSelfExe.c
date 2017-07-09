// retrieve the full path of the current executable
// see binreloc.c for a more advanced variant

#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *getSelfExe()
{
  char buf[PATH_MAX];
  const char *path;
  ssize_t size;

  path = "/proc/self/exe";
  size = readlink(path, buf, PATH_MAX);

  if (size == -1)
  {
    return NULL;
  }
  buf[size] = '\0';

  return strdup(buf);
}

int main(void)
{
  char *self = getSelfExe();

  if (self == NULL)
  {
    return 1;
  }

  printf("%s\n", self);
  return 0;
}

