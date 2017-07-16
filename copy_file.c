#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* see cp(1) for more information */
enum {
  CP_FORCE     = 1 << 1, /* force overwriting an existing destination file */
  CP_NODEREF   = 1 << 2, /* never follow symbolic links in source */
  CP_TARGETDIR = 1 << 3, /* copy source into dest directory */
  CP_DIR       = 1 << 4  /* treat source as a directory and copy it with all its contents */
};

/* fork /bin/cp and return the pid; set flags to 0 for default settings */
pid_t copy_file_start(const char *source, const char *dest, int flags)
{
  const char *opts;
  pid_t pid, cf_err = 0;

  if (source == NULL || strlen(source) == 0 || dest == NULL || strlen(dest) == 0)
  {
    return cf_err;
  }

  /* source does not exist */
  if (access(source, F_OK) != 0)
  {
    return cf_err;
  }

  /* target exists */
  if (access(dest, F_OK) == 0)
  {
    if (!((flags & CP_TARGETDIR) || (flags & CP_FORCE)))
    {
      /* we didn't set CP_TARGETDIR or CP_FORCE */
      return cf_err;
    }
    else if ((flags & CP_TARGETDIR) && !(flags & CP_FORCE))
    {
      /* we set CP_TARGETDIR but not CP_FORCE */
      char *source_base = basename((char *)source);
      char target_dest[strlen(dest) + strlen(source_base) + 2];
      sprintf(target_dest, "%s/%s", dest, source_base);

      /* path already exists inside targetdir */
      if (access(target_dest, F_OK) == 0)
      {
        return cf_err;
      }
    }
  }

  if (flags & CP_NODEREF)
  {
    opts = (flags & CP_DIR) ? "-rfP" : "-fP";
  }
  else
  {
    opts = (flags & CP_DIR) ? "-rfL" : "-fL";
  }

  pid = fork();

  if (pid == 0)
  {
    if (flags & CP_TARGETDIR)
    {
      execl("/bin/cp", "cp", opts, source, "-t", dest, (char *)0);
    }
    else
    {
      execl("/bin/cp", "cp", opts, "-T", source, dest, (char *)0);
    }
    _exit(1);
  }

  return pid;
}

/* return 0 on success and 1 on error */
int get_pid_status_simple(pid_t pid)
{
  int status;

  if (pid > 0 && waitpid(pid, &status, 0) > 0 && WIFEXITED(status) == 1 && WEXITSTATUS(status) == 0)
  {
    return 0;
  }
  return 1;
}

/* simple file copying function */
#define copy_file(source,dest,flags)  get_pid_status_simple(copy_file_start(source, dest, flags))
//int copy_file(const char *source, const char *dest, int flags)
//{
//  return get_pid_status_simple(copy_file_start(source, dest, flags));
//}

int main(void)
{
  /*
  pid_t pid = copy_file_start("source", "dest", CP_DIR);
  printf("pid: %d\n", (int)pid);
  if (pid > 0)
  {
    int status = get_pid_status_simple(pid);
    printf("pid returned status: %d\n", status);
  }
  return 0;
  */
  return copy_file("source", "dest/", CP_DIR);
}

