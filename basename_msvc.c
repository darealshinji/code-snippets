/**
 * Quick and simple MSVC implementation of basename() and
 * dirname() using _splitpath_s().
 * Not exaclty like the real POSIX functions, but close enough.
 */

#ifndef _MSC_VER
#error Not using MSVC
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *basename(char *path)
{
  char fname[_MAX_FNAME];
  char ext[_MAX_EXT];

  if (_splitpath_s(path, NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT) != 0)
  {
    return NULL;
  }

  _snprintf_s(path, strlen(path), _TRUNCATE, "%s%s", fname, ext);

  return path;
}

char *dirname(char *path)
{
  char drive[_MAX_DRIVE];
  char dir[_MAX_DIR];

  if (_splitpath_s(path, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0) != 0)
  {
    return NULL;
  }

  _snprintf_s(path, strlen(path), _TRUNCATE, "%s%s", drive, dir);

  return path;
}

int main(int argc, char *argv[])
{
  char *orig = "C:\\DirA\\DirB\\file.ext";
  char *base = basename(strdup(orig));
  char *dir = dirname(strdup(orig));

  printf("original:\n%s\n\n", orig);
  printf("basename:\n%s\n\n", base);
  printf("dirname:\n%s\n", dir);

  free(base);
  free(dir);
  return 0;
}
