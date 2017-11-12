#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* retrieve the full path of the current executable */

int main(void)
{
  const char *self = "/proc/self/exe";
  char path[PATH_MAX + 1] = {0};

  char *rp = realpath(self, path);
  int errsv = errno;

  if (rp)
  {
    printf("%s\n", path);
  }
  else
  {
    fprintf(stderr, "%s(): %s\n", __func__, strerror(errsv));
    return 1;
  }

  return 0;
}

