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

#include <limits.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef PROG
# define PROG "prog"
#endif

#define PRINT_ERROR(msg) fprintf(stderr, "error: " msg " \n")

char getSelfExeBuf[PATH_MAX + 1];

char *getSelfExe()
{
  memset(getSelfExeBuf, '\0', PATH_MAX + 1);
  ssize_t size = readlink("/proc/self/exe", getSelfExeBuf, PATH_MAX);

  if (size == -1)
  {
    return NULL;
  }
  return getSelfExeBuf;
}

int runCommand(const char *command)
{
  int status;
  int rv = 127;
  pid_t pid = fork();

  if (pid == 0)
  {
    //execl("/bin/sh", "sh", "-c", command, (char *)NULL);
    execl(command, command, (char *)NULL);
    _exit(127);  /* if execl() was successful, this won't be reached */
  }

  if (pid > 0)
  {
    if (waitpid(pid, &status, 0) > 0)
    {
      if (WIFEXITED(status) == 1)
      {
        if ((rv = WEXITSTATUS(status)) == 127)
        {
          PRINT_ERROR("execl() failed");
        }
      }
      else
      {
        PRINT_ERROR("the program did not terminate normally");
      }
    }
    else
    {
      PRINT_ERROR("waitpid() failed");
    }
  }
  else
  {
    PRINT_ERROR("failed to fork()");
  }

  return rv;
}

/**
 * Move into executable directory, get the machine type and
 * run "./PROG.x86_64" on x86_64, otherwise "./PROG.x86"
 */
int main(void)
{
  struct utsname sysInfo;
  char *self = getSelfExe();
  int rv = 127;

  if (self != NULL && chdir(dirname(self)) != 0)
  {
    PRINT_ERROR("chdir()");
  }

  if (uname(&sysInfo) == 0 && strcmp("x86_64", sysInfo.machine) == 0)
  {
    rv = runCommand("./" PROG ".x86_64");
  }
  else
  {
    rv = runCommand("./" PROG ".x86");
  }

  return rv;
}

