#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* remove files, links, empty and full directories by forking /bin/rm;
 * set ignoreNonExistent to non-zero if non-existent paths should be ignored;
 * see also: remove(3), unlink(2), rmdir(2), rm(1) */
pid_t remove_recursive_start(const char *path, int ignoreNonExistent)
{
  pid_t pid = 0;

  if (path == NULL || strlen(path) == 0)
  {
    return pid;
  }

  pid = fork();

  if (pid == 0)
  {
    const char *opts = (ignoreNonExistent == 0) ? "-r" : "-rf";
    execl("/bin/rm", "rm", opts, path, (char *)0);
    _exit(1);
  }

  return pid;
}

/* return 0 on success and 1 on error */
int get_pid_status_simple(pid_t pid)
{
  int status;

  if (pid > 0 && waitpid(pid, &status, 0) > 0 &&
      WIFEXITED(status) == 1 && WEXITSTATUS(status) == 0)
  {
    return 0;
  }
  return 1;
}

/* simple file removing function */
int remove_recursive(const char *path, int ignoreNonExistent)
{
  /* try remove(3) before we fork(2) a process */
  if (remove(path) == 0)
  {
    return 0;
  }
  pid_t pid = remove_recursive_start(path, ignoreNonExistent);
  return get_pid_status_simple(pid);
}

int main(void)
{
  return remove_recursive("remove_this", 0);
}

