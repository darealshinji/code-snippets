/**
 * simple implementation of Debian's "which"
 * command in C (see Debian's "debianutils" package)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *which(const char *command)
{
  size_t len;
  char *env, *str, *token;

  if (!command || (len = strlen(command)) == 0 || (env = getenv("PATH")) == NULL) {
    return NULL;
  }

  str = env = strdup(env);

  while ((token = strtok(str, ":")) != NULL) {
    char *file = malloc(strlen(token) + len + 2);
    sprintf(file, "%s/%s", token, command);

    if (access(file, R_OK|X_OK) == 0) {
      free(env);
      return file;
    }

    free(file);
    str = NULL;
  }

  free(env);
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
    if ((path = which(argv[i])) != NULL) {
      rv = 0;
      printf("%s\n", path);
      free(path);
    }
  }
  return rv;
}

