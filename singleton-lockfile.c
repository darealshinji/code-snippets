#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/file.h>

// use a unique filename
#define LOCKFILE  "/dev/shm/.dd02686a-2b36-477d-977c-696e2a363ead.lock"

static int fd_lockfile = -1;

static bool lock_app()
{
  fd_lockfile = open(LOCKFILE, O_CREAT|O_CLOEXEC|O_RDONLY, 0444);

  if (fd_lockfile == -1 || flock(fd_lockfile, LOCK_EX|LOCK_NB) == -1) {
    return false;
  }

  return true;
}

static void release_app()
{
  if (flock(fd_lockfile, LOCK_UN) == 0) {
    unlink(LOCKFILE);
  }
}


int main()
{
  if (lock_app() == false) {
    if (errno = EWOULDBLOCK) {
      printf("another instance is already running\n");
    }
    return 1;
  }

  printf("first instance started\n");
  sleep(5);
  printf("first instance stopped\n");

  release_app();

  return 0;
}
