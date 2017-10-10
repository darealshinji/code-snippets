// retrieve the full path of the current executable
// see binreloc.c for a more advanced variant

#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

char getSelfExeBuf[PATH_MAX + 1];

char *getSelfExe()
{
  memset(getSelfExeBuf, '\0', PATH_MAX + 1);
  ssize_t size = readlink("/proc/self/exe", getSelfExeBuf, PATH_MAX);

  if (size == -1)
  {
    return NULL;
  }
  return getSelfExeBuf;
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

