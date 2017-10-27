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
#define BUF_SIZE  PATH_MAX + 1

#ifndef ALLOCATE_BUFFER
static char _getSelfExeBuf[BUF_SIZE];
#endif

char *getSelfExe()
{
  char *buf;
  ssize_t size;

#ifdef ALLOCATE_BUFFER
  buf = malloc(BUF_SIZE);
#else
  buf = _getSelfExeBuf;
#endif
  size = readlink("/proc/self/exe", buf, BUF_SIZE);

  if (size == -1)
  {
#ifdef ALLOCATE_BUFFER
    free(buf);
#endif
    return NULL;
  }
  else if (size == BUF_SIZE)
  {
    errno = ENAMETOOLONG;
    buf[PATH_MAX] = '\0';
  }
  else
  {
    memset(buf + size, '\0', BUF_SIZE - size);
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

