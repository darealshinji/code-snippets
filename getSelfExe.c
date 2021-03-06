#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* retrieve the full path of the current executable on Linux */

int main(void)
{
  char *rp = realpath("/proc/self/exe", NULL);
  int errsv = errno;

  if (rp)
  {
    printf("%s\n", rp);
    free(rp);
  }
  else
  {
    fprintf(stderr, "%s(): %s\n", __func__, strerror(errsv));
    return 1;
  }

  return 0;
}

