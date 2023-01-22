#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>


int mkdir_p(const char *path)
{
  char buf[PATH_MAX] = {0};
  const char *p;
  char *pb;
  char peek;

  /* invalid argument */
  if (!path || *path == 0) {
    errno = EINVAL;
    return -1;
  }

  /* path too long */
  if (strlen(path) >= PATH_MAX) {
    errno = ENAMETOOLONG;
    return -1;
  }

  buf[0] = path[0];
  pb = buf;

  /* copy path and call mkdir() on each path separator */
  for (p = path + 1; *p; p++) {
    if (*p == '/') {
      if ((peek = *(p+1)) == '/' || peek == 0) {
        /* skip multiple or trailing slashes */
        continue;
      }
      *(pb+1) = 0;

      if (mkdir(buf, S_IRWXU) != 0 && errno != EEXIST) {
        /* ignore "directory exists" error */
        return -1;
      }
    }
    *++pb = *p;
  }
  *++pb = 0;

  if (mkdir(buf, S_IRWXU) != 0 && errno != EEXIST) {
    return -1;
  }

  errno = 0;
  return 0;
}

int main()
{
  const char *path = "/tmp/a/b/c";
  printf("create path: %s\n", path);

  if (mkdir_p(path) == -1) {
    perror("mkdir_p()");
    return 1;
  }

  puts("success!");
  return 0;
}
