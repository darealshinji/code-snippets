#include <sys/stat.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>

#include "plugin.h"

#define PLUGIN      libqtfilename_so
#define PLUGIN_LEN  libqtfilename_so_len

int load_plugin(unsigned char *data, unsigned int len, int *rv, int argc, char **argv)
{
  FILE *fp;
  char file[256];
  unsigned char buf[4];

  long pid = (long)getpid();

  // get random bytes

  if ((fp = fopen("/dev/urandom", "r")) == NULL) {
    goto JMP_DEF;
  }

  if (fread(buf, 1, sizeof(buf), fp) != sizeof(buf)) {
    fclose(fp);
    goto JMP_DEF;
  }

  fclose(fp);
  sprintf(file, "/tmp/qt_%ld_%02x%02x%02x%02x", pid, buf[0], buf[1], buf[2], buf[3]);

  goto JMP_CREAT;

  // write plugin to /dev/shm

JMP_DEF:
  sprintf(file, "/tmp/qt_%ld", pid);

JMP_CREAT:
  if ((fp = fopen(file, "w")) == NULL) {
    perror("fopen()");
    return 1;
  }

  if (fwrite(data, 1, len, fp) != len) {
    perror("fwrite()");
    fclose(fp);
    unlink(file);
    return 1;
  }

  fclose(fp);
  chmod(file, S_IRUSR);

  // load plugin

  void *handle = dlopen(file, RTLD_NOW);
  char *error = dlerror();

  if (!handle) {
    fprintf(stderr, "%s\n", error ? error : "dlopen() has returned an error");
    unlink(file);
    return 1;
  }

  dlerror();

  int (*libmain)(int, char **, int);
  libmain = dlsym(handle, "libmain");

  if ((error = dlerror()) != NULL) {
    fprintf(stderr, "%s\n", error);
    dlclose(handle);
    unlink(file);
    return 1;
  }

  *rv = libmain(argc, argv, 1);

  dlclose(handle);
  unlink(file);

  return 0;
}

int main(int argc, char **argv)
{
  int rv = 0;

  if (load_plugin(PLUGIN, PLUGIN_LEN, &rv, argc, argv) != 0) {
    fprintf(stderr, "function load_plugin() returned with an error\n");
    return 1;
  }

  return rv;
}


