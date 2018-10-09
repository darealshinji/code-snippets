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

/* fork cp(1) and return the pid; set flags to 0 for default settings */
pid_t copy_file_start(const char *source, const char *dest, int flags)
{
  char opts[5] = { '-', 'f', 'L', '\0', '\0' };
  pid_t pid;

  if (source == NULL || dest == NULL || strlen(source) == 0 || strlen(dest) == 0) {
    return 0;
  }

  /* source does not exist */
  if (access(source, F_OK) != 0) {
    return 0;
  }

  /* target exists */
  if (access(dest, F_OK) == 0) {
    if ( !((flags & CP_TARGETDIR) || (flags & CP_FORCE)) ) {
      return 0;
    } else if ( (flags & CP_TARGETDIR) && !(flags & CP_FORCE) ) {
      char do_return = 0;
      char *copy = strdup(source);
      char *source_base = basename(copy);
      char *target_dest = malloc(strlen(dest) + strlen(source_base) + 1);
      sprintf(target_dest, "%s/%s", dest, source_base);

      /* path already exists inside targetdir */
      if (access(target_dest, F_OK) == 0) {
        do_return = 1;
      }
      free(copy);
      free(target_dest);

      if (do_return == 1) {
        return 0;
      }
    }
  }

  if (flags & CP_NODEREF) {
    opts[2] = 'P';
  }

  if (flags & CP_DIR) {
    opts[3] = 'r';
  }

  if ((pid = fork()) == 0) {
    if (flags & CP_TARGETDIR) {
      execlp("cp", "cp", opts, source, "-t", dest, NULL);
    } else {
      execlp("cp", "cp", opts, "-T", source, dest, NULL);
    }
    _exit(1);
  }

  return pid;
}

/* return 0 on success and 1 on error */
int get_pid_status_simple(pid_t pid)
{
  int status;

  if (pid > 0 && waitpid(pid, &status, 0) > 0 && WIFEXITED(status) == 1 && WEXITSTATUS(status) == 0) {
    return 0;
  }
  return 1;
}

/* simple file copying function */
#define copy_file(source,dest,flags)  get_pid_status_simple(copy_file_start(source,dest,flags))
//int copy_file(const char *source, const char *dest, int flags) {
//  return get_pid_status_simple(copy_file_start(source, dest, flags));
//}

int main(void)
{
  //return copy_file("source", "dest", CP_DIR);
  pid_t pid = copy_file_start("source", "dest", CP_DIR);
  printf("pid == %d\n", pid);
  if (pid > 0) {
    printf("pid returned status: %d\n", get_pid_status_simple(pid));
  } else {
    printf("error\n");
    return 1;
  }
  return 0;
}

