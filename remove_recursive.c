#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

pid_t rr_ok = -1;
pid_t rr_err = 0;

/* remove files, links, empty and full directories by forking /bin/rm;
 * set ignoreNonExistent to non-zero if non-existent paths should be ignored;
 * see also: remove(3), unlink(2), rmdir(2), rm(1) */
pid_t remove_recursive_start(const char *path, int ignoreNonExistent)
{
  if (path == NULL || strlen(path) == 0)
  {
    return rr_err;
  }

  /* path does not exist */
  if (access(path, F_OK) != 0)
  {
    return (ignoreNonExistent == 0) ? rr_err : rr_ok;
  }

  /* try remove(3) before we fork(2) a process */
  if (remove(path) == 0)
  {
    return rr_ok;
  }

  pid_t pid = fork();

  if (pid == 0)
  {
    execl("/bin/rm", "rm", "-r", path, (char *)0);
    _exit(1);
  }

  return pid;
}

/* return 0 on success and 1 on error */
int get_pid_status_simple(pid_t pid)
{
  int status;

  if (pid == rr_ok)
  {
    return 0;
  }
  else if (pid == rr_err)
  {
    return 1;
  }
  else if (pid > 0 && waitpid(pid, &status, 0) > 0 && WIFEXITED(status) == 1 && WEXITSTATUS(status) == 0)
  {
    return 0;
  }

  return 1;
}

/* simple file removing function */
#define remove_recursive(path,ignoreNonExistent)  get_pid_status_simple(remove_recursive_start(path, ignoreNonExistent))
//int remove_recursive(const char *path, int ignoreNonExistent)
//{
//  return get_pid_status_simple(remove_recursive_start(path, ignoreNonExistent));
//}

int main(void)
{
  /*
  pid_t pid = remove_recursive_start("remove_this", 0);
  printf("pid: %d\n", (int)pid);
  if (pid > 0)
  {
    int status = get_pid_status_simple(pid);
    printf("pid returned status: %d\n", status);
  }
  return 0;
  */
  return remove_recursive("remove_this", 0);
}

