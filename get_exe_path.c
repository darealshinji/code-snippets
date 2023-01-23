#if defined(USE_WHEREAMI)
// https://github.com/gpakosz/whereami
#  include "whereami/src/whereami.h"
#  include "whereami/src/whereami.c"
#elif defined(_WIN32)
#  include <windows.h>
#  include <string.h>
#  include <wchar.h>
#elif defined(__APPLE__)
#  include <sys/syslimits.h>
#  include <mach-o/dyld.h>
#  include <string.h>
#else
#  include <limits.h>
#  include <stdlib.h>
#endif

#include <stdlib.h>
#include <stdio.h>


char *get_exe_path()
{
#if defined(USE_WHEREAMI)
  /* handles many OSes and tries to get around MAX_PATH limits */
  char *path;
  int len = wai_getExecutablePath(NULL, 0, NULL);
  if (len == -1) return NULL;
  path = (char *)malloc(len + 1);
  wai_getExecutablePath(path, len, NULL);
  path[len] = '\0';
  return path;
#elif defined(_WIN32)
  /* 32k is only for wide char API I think, but whatever ... */
  char buf[32*1024] = {0};
  if (GetModuleFileNameA(NULL, buf, sizeof(buf)-1) > 0 && GetLastError() == 0) {
    buf[0] = 0;
  }
  return (buf[0] == 0) ? NULL : strdup(buf);
#elif defined(__APPLE__)
  /* not using realpath() ... */
  char buf[PATH_MAX];
  uint32_t len = sizeof(buf)-1;
  if (_NSGetExecutablePath(buf, &len) != 0) return NULL;
  return strdup(buf);
#else
  /* assuming Linux; GLIBC and syscalls are
   * usually still limited to MAX_PATH */
  return realpath("/proc/self/exe", NULL);
#endif
}

#if defined(_WIN32)
wchar_t *get_exe_pathW()
{
  wchar_t buf[32*1024] = {0};
  if (GetModuleFileNameW(NULL, buf, sizeof(buf)-1) > 0 && GetLastError() == 0) {
    buf[0] = 0;
  }
  return (buf[0] == 0) ? NULL : wcsdup(buf);
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
