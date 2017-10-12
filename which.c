/**
 * simple implementation of Debian's /bin/which
 * command in C (see Debian's "debianutils" package)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
  char *env, *env_copy, *str, *token, *file;
  int i, j, len, rv = 0;

  if (argc < 2)
  {
    return 1;
  }

  env = getenv("PATH");

  if (!env)
  {
    fprintf(stderr, "error: environment variable PATH not found!\n");
    return 1;
  }

  for (i = 1; i < argc; ++i)
  {
    env_copy = strdup(env);

    /**
     * though not really needed, stop after 9999 iterations
     * to prevent any potential endless loop
     */
    for (j = 0, str = env_copy; j < 9999; ++j, str = NULL)
    {
      len = strlen(argv[i]);
      token = strtok(str, ":");

      if (token == NULL || len == 0)
      {
        rv = 1;
        break;
      }

      file = malloc(strlen(token) + len + 2);
      sprintf(file, "%s/%s", token, argv[i]);

      if (access(file, R_OK|X_OK) == 0)
      {
        printf("%s\n", file);
        free(file);
        break;
      }
    }

    free(env_copy);
  }

  return rv;
}

