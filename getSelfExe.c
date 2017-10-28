// retrieve the full path of the current executable

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* testing */
//#undef PATH_MAX
//#define PATH_MAX 9

//#define ALLOCATE_BUFFER

#ifndef ALLOCATE_BUFFER
static char _getSelfExeBuf[PATH_MAX + 1];
#endif

char *getSelfExe()
{
  char *buf;
  ssize_t size;

#ifdef ALLOCATE_BUFFER
  buf = malloc(PATH_MAX + 1);
#else
  buf = _getSelfExeBuf;
#endif
  memset(buf, '\0', PATH_MAX + 1);
  size = readlink("/proc/self/exe", buf, PATH_MAX + 1);

  if (size == -1)
  {
#ifdef ALLOCATE_BUFFER
    free(buf);
#endif
    return NULL;
  }

  if (buf[PATH_MAX] != '\0')
  {
    errno = ENAMETOOLONG;
    buf[PATH_MAX] = '\0';
  }

  return buf;
}

int main(void)
{
  char *self = getSelfExe();
  int errno_returned = errno;

  if (self == NULL)
  {
    return 1;
  }

  if (errno_returned == ENAMETOOLONG)
  {
    fprintf(stderr, "warning: path length exceeds %d bytes and was truncated!\n", PATH_MAX);
  }
  printf("path: %s\n", self);
  printf("length: %ld\n", strlen(self));

#ifdef ALLOCATE_BUFFER
  free(self);
#endif
  return 0;
}

/* a more simple example */
/*
int main(void)
{
  char buf[PATH_MAX + 1] = {0};
  if (readlink("/proc/self/exe", buf, PATH_MAX) == -1) {
    return 1;
  }
  printf("%s\n", buf);
  return 0;
}
*/

