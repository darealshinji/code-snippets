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

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


// this is a wrapper around the wslpath command
//
// path   -  input path
// buf    -  buffer to store translated path
// szBuf  -  size of buffer
//
// mode:
//   "-u" or "u"  -  Windows path to Unix path
//   "-w" or "w"  -  Unix path to Windows path
//   "-m" or "m"  -  Unix path to Windows path, using '/' as path separator
//   any other arguments (including NULL) will be treated like "-u"
//
// absolute - set non-zero to force result to absolute path format
//
char *wslpath(const char *path, char *buf, size_t szBuf, const char *mode, int absolute);


FILE *popen_wslpath(const char *mode, const char *path, int readErr, int absolute)
{
  enum { r = 0, w = 1 };
  int fd[2];
  int fdnull;
  pid_t pid;
  char *argv[5] = { "wslpath", NULL, NULL, NULL, NULL };

  if (pipe2(fd, O_CLOEXEC) == -1) {
    //perror("pipe2()");
    return NULL;
  }

  if ((pid = fork()) == -1) {
    //perror("fork()");
    return NULL;
  }

  if (pid != 0) {
    // parent process reads from pipe
    close(fd[w]);
    return fdopen(fd[r], "r");
  }

  // child process writes to pipe

  fdnull = open("/dev/null", O_WRONLY|O_CLOEXEC);

  if (readErr == 0) {
    dup2(fd[w], STDOUT_FILENO);  // read stdout
    dup2(fdnull, STDERR_FILENO); // silence stderr
  } else {
    dup2(fd[w], STDERR_FILENO);  // read stderr
    dup2(fdnull, STDOUT_FILENO); // silence stdout
  }

  close(fd[r]);
  close(fd[w]);

  if (absolute == 0) {
    argv[1] = (char * const) mode;
    argv[2] = (char * const) path;
  } else {
    argv[1] = "-a";  // does this even have any effect on the output at all?
    argv[2] = (char * const) mode;
    argv[3] = (char * const) path;
  }

  execvp("wslpath", argv);
  _exit(127);

  return NULL;
}

char *wslpath(const char *path, char *buf, size_t szBuf, const char *mode, int absolute)
{
  FILE *fp;
  size_t len;
  int errsv = 0;
  const char *arg = "-u";

  if (!path || strlen(path) == 0 || !buf || szBuf < 1) {
    errno = EINVAL;  // Invalid argument
    return NULL;
  }

  if (mode) {
    if (strcasecmp(mode, "w") == 0 || strcasecmp(mode, "-w") == 0) {
      arg = "-w";
    } else if (strcasecmp(mode, "m") == 0 || strcasecmp(mode, "-m") == 0) {
      arg = "-m";
    }
  }

  // unfortunately wslpath always returns 0 and prints help to stdout on error;
  // to figure out if an error occured, redirect stdout to /dev/null
  // and read from stderr; if nothing was read, then no error occurred and
  // we can do the same again, now reading the result from stdin

  // check stderr
  if ((fp = popen_wslpath(arg, path, 1, absolute)) == NULL) {
    return NULL;
  }

  if (fread(buf, 1, szBuf, fp) > 0 || !feof(fp) || ferror(fp)) {
    // wslpath returned a message to stderr
    pclose(fp);
    errno = ECANCELED;  // Operation canceled
    return NULL;
  }

  // now read from stdin
  if ((fp = popen_wslpath(arg, path, 0, absolute)) == NULL) {
    return NULL;
  }

  if ((len = fread(buf, 1, szBuf, fp)) > 0 && feof(fp) && !ferror(fp)) {
    // remove trailing newline
    buf[len - 1] = 0;
  } else {
    errsv = (errno == 0) ? -1 : errno;
    buf = NULL;
  }

  pclose(fp);
  errno = errsv;

  return buf;
}

int main()
{
  const char *input = "\\\\wsl$\\Ubuntu\\usr\\bin\\..";
  char buf[2048];

  if (wslpath(input, buf, sizeof(buf) - 1, "u", 1)) {
    printf("input:   %s\n", input);
    printf("result:  %s\n", buf);
  } else {
    perror("wslpath()");
    return 1;
  }

  return 0;
}
