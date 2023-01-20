#if defined(USE_WHEREAMI)
// https://github.com/gpakosz/whereami
#  include "whereami/src/whereami.h"
#  include "whereami/src/whereami.c"
#elif defined(_WIN32)
#  include <windows.h>
#elif defined(__APPLE__)
#  include <limits.h>
#  include <mach-o/dyld.h>
#else
#  include <limits.h>
#  define GET_EXE_PATH_INLINE inline
#endif

#if !defined(GET_EXE_PATH_INLINE)
#  define GET_EXE_PATH_INLINE
#endif

#include <stdlib.h>
#include <stdio.h>


static GET_EXE_PATH_INLINE
char *get_exe_path()
{
#if defined(USE_WHEREAMI)
  char *path;
  int len = wai_getExecutablePath(NULL, 0, NULL);
  if (len == -1) return NULL;
  path = (char *)malloc(len + 1);
  wai_getExecutablePath(path, len, NULL);
  path[len] = '\0';
  return path;
#elif defined(_WIN32)
  DWORD len = 4096; //32*1024;
  char *buf = (char *)malloc(len);
  if (GetModuleFileNameA(NULL, buf, len) > 0 && GetLastError() == 0) {
    return buf;
  }
  free(buf);
  return NULL;
#elif defined(__APPLE__)
  char *path;
  char buf[4096];
  uint32_t len = sizeof(buf);
  if (_NSGetExecutablePath(buf, &len) != 0) return NULL;
  path = (char *)malloc(sizeof(buf));
  if (realpath(buf, path) != NULL) return path;
  free(path);
  return NULL;
#else
  return realpath("/proc/self/exe", NULL);
#endif
}


int main()
{
  char *path = get_exe_path();
  if (!path) return 1;
  printf("%s\n", path);
  free(path);

  return 0;
}
