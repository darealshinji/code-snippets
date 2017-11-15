// useful to avoid issues with problematic characters in
// filenames such as spaces or apostrophs

FILE *popen_gzip(const char *file)
{
  enum { r = 0, w = 1 };
  int fd[2];

  if (pipe(fd) == -1)
  {
    perror("pipe()");
    return NULL;
  }

  if (fork() == 0)
  {
    close(fd[r]);
    dup2(fd[w], 1);
    close(fd[w]);
    execl("/bin/gzip", "gzip", "-cd", file, NULL);
    _exit(127);
  }
  else
  {
    close(fd[w]);
    return fdopen(fd[r], "r");
  }

  return NULL;
}
