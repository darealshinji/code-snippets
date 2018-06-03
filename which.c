/**
 * simple implementation of Debian's /bin/which
 * command in C (see Debian's "debianutils" package)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *which(const char *command)
{
  size_t len, i;
  char *env, *str;

  if (!command || (len = strlen(command)) == 0 || (env = getenv("PATH")) == NULL) {
    return NULL;
  }

  /**
   * though not really needed, stop after 999 iterations
   * to prevent any potential endless loop
   */
  for (i = 0, str = env; i < 999; ++i, str = NULL) {
    char *token = strtok(str, ":");

    if (!token) {
      return NULL;
    }

    char *file = malloc(strlen(token) + len + 2);
    sprintf(file, "%s/%s", token, command);

    if (access(file, R_OK|X_OK) == 0) {
      return file;
    }
    free(file);
  }

  return NULL;
}

int main(int argc, char *argv[])
{
  char *path;
  int rv = 1;

  if (argc < 2) {
    return 1;
  }

  for (int i = 1; i < argc; i++) {
    path = which(argv[i]);
    if (path) {
      rv = 0;
      printf("%s\n", path);
      free(path);
    }
  }
  return rv;
}

