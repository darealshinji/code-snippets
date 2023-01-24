#include <stdlib.h>
#include <stdio.h>
#if !defined(__linux__) && !defined(__CYGWIN__)
// https://github.com/gpakosz/whereami
#include "whereami/src/whereami.h"
#endif

#if defined(__linux__) || defined(__CYGWIN__)

inline char *get_exe_path()
{
  return realpath("/proc/self/exe", NULL);
}

#else

#include "whereami/src/whereami.c"

char *get_exe_path()
{
  char *path;
  int len = wai_getExecutablePath(NULL, 0, NULL);
  if (len == -1) return NULL;
  path = (char *)malloc(len + 1);
  wai_getExecutablePath(path, len, NULL);
  path[len] = '\0';
  return path;
}

#endif


int main()
{
  char *path = get_exe_path();
  if (!path) return 1;
  printf("%s\n", path);
  free(path);

  return 0;
}
