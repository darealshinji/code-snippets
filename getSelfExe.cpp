#include <iostream>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* retrieve the full path of the current executable on Linux */

int main(void)
{
  char *rp = realpath("/proc/self/exe", NULL);
  int errsv = errno;

  std::string resolved_path = rp ? std::string(rp) : "";
  free(rp);

  if (!resolved_path.empty())
  {
    std::cout << resolved_path << std::endl;
  }
  else
  {
    std::cerr << __func__ << "(): " << strerror(errsv) << std::endl;
    return 1;
  }

  return 0;
}

