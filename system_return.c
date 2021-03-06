#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * like system() but it returns the exit code of the
 * given command rather than that of the forked shell
 */

#define PRINT_ERROR(msg) fprintf(stderr, "error: " msg " \n")

int system_return(const char *command)
{
  int status;
  int rv = 127;
  pid_t pid = fork();

  if (pid == 0)
  {
    execl("/bin/sh", "sh", "-c", command, (char *)NULL);
    //execl(command, command, (char *)NULL);
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

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    fprintf(stderr, "usage: %s command\n", argv[0]);
    return 1;
  }

  int rv = system_return(argv[1]);
  printf("command `%s' returned %d\n", argv[1], rv);

  return 0;
}

