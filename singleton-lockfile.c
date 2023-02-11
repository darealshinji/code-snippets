#include <sys/file.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

// use a unique filename
#define LOCKFILE "/dev/shm/.dd02686a-2b36-477d-977c-696e2a363ead.lock"

static int fd_lockfile = -1;

static int lock_app()
{
  fd_lockfile = open(LOCKFILE, O_CREAT | O_CLOEXEC | O_RDWR, 0666);

  if (fd_lockfile == -1) return -1;

  if (flock(fd_lockfile, LOCK_EX | LOCK_NB) == -1) {
    if (errno = EWOULDBLOCK) {
      printf("another instance is already running\n");
      return -1;
    }
    perror("flock()");
    return -1;
  }

  return 0;
}

static void release_app()
{
  if (flock(fd_lockfile, LOCK_UN) == 0) {
    unlink(LOCKFILE);
  }
}


int main()
{
  if (lock_app() == -1) return 1;

  printf("first instance started\n");
  sleep(5);
  printf("first instance stopped\n");

  release_app();
  return 0;
}
