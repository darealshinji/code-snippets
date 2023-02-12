#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


static int lock_app(const char *pidfile)
{
  int fd = open(pidfile, O_RDWR);

  if (fd == -1 || flock(fd, LOCK_EX | LOCK_NB) == -1) {
    return -1;
  }

  return fd;
}

static void release_app(int fd, const char *pidfile)
{
  if (flock(fd, LOCK_UN) == 0) {
    unlink(pidfile);
  }
}

static bool pidfile_exists(const char *pidfile)
{
  struct stat st;

  if (stat(pidfile, &st) == 0 && S_ISREG(st.st_mode)) {
    return true;
  }

  return false;
}

static bool create_pidfile(const char *pidfile, bool *first_instance)
{
  char buf[32] = {0};
  size_t len;
  ssize_t n;
  int fd;

  if ((fd = open(pidfile, O_CREAT | O_RDWR, 0666)) == -1) {
    return false;
  }

  sprintf(buf, "%d", getpid());
  len = strlen(buf);

  n = write(fd, buf, len);
  close(fd);

  if (n == len) {
    *first_instance = true;
    return true;
  }

  return false;
}

static int check_first_instance(const char *pidfile)
{
  char buf[32] = {0};
  char path[1024] = {0};
  char path2[1024] = {0};
  char pidfile_res[4096] = {0};
  char res[4096] = {0};
  DIR *dirp;
  int fd;
  ssize_t rd;
  struct dirent *entry;
  bool first_instance = false;

  if (pidfile_exists(pidfile) == false &&
      create_pidfile(pidfile, &first_instance) == false)
  {
    return -1;
  }

  if (first_instance == true) {
    return 0;
  }

  // read PID from file

  if ((fd = open(pidfile, O_RDONLY)) == -1) {
    return -1;
  }

  rd = read(fd, &buf, sizeof(buf)-1);
  close(fd);

  // error
  if (rd == -1) return -1;

  // empty file
  if (rd == 0) {
    create_pidfile(pidfile, &first_instance);
    return 0;
  }

  // check if PID has opened the file

  snprintf(path, sizeof(path)-1, "/proc/%s/fd", buf);

  if ((dirp = opendir(path)) == NULL) {
    create_pidfile(pidfile, &first_instance);
    return 0;
  }

  if (realpath(pidfile, pidfile_res) == NULL) {
    return -1;
  }

  while ((entry = readdir(dirp)) != NULL)
  {
    if (entry->d_type == DT_DIR) continue;
    snprintf(path2, sizeof(path2)-1, "%s/%s", path, entry->d_name);
    if (realpath(path2, res) == NULL) continue;

    if (strcmp(res, pidfile_res) == 0) {
      //printf("`%s' -> `%s'\n", path2, res);
      int pid = atoi(buf);
      closedir(dirp);
      return (pid == 0) ? 1 : pid;
    }
  }

  closedir(dirp);
  create_pidfile(pidfile, &first_instance);

  return 0;
}


int main()
{
  const char *pidfile = "/dev/shm/.83e3e570-31a5-4fc9-b326-8f2cfb85fbdf.pid";

  int pid = check_first_instance(pidfile);

  switch (pid) {
    case 0:
      // we are the first instance
      break;
    case -1:
      puts("an error has occurred");
      return 1;
    default:
      printf("another instance is already running: %d\n", pid);
      return 1;
  }

  int fd = lock_app(pidfile);

  if (fd == -1) {
    puts("locking failed");
    return 1;
  }

  puts("first instance");
  sleep(15);

  release_app(fd, pidfile);

  return 0;
}
