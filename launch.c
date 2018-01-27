/**
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <http://unlicense.org/>
 */

/**
 * Decide on runtime whether to start a 32 bit or 64 bit program.
 * Useful for Unity 3D games. Compile with "-m32 -static".
 */

#include <libgen.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>

#ifndef PROG
# define PROG "prog"
#endif

int main(void)
{
  struct utsname sysinfo;
  const char *prog = PROG ".x86";
  char *self, *dn, *path;

  if ((self = realpath("/proc/self/exe", NULL)) == NULL) {
    perror("realpath()");
    return 1;
  }

  dn = dirname(self);
  free(self);

  if (uname(&sysinfo) == 0 && strcmp("x86_64", sysinfo.machine) == 0) {
    prog = PROG ".x86_64";
  }

  path = malloc(strlen(dn) + 1 + strlen(prog));
  sprintf(path, "%s/%s", dn, prog);

  execl(path, path, (char *)NULL);
  perror(path);
  free(path);

  return 1;
}

