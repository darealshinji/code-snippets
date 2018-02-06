#include <stdlib.h>
#include <dlfcn.h>

//#define MODULE "simple_message.so"
#define MODULE "simple_message64.so"

int main(void)
{
  typedef void (*msg_t)(const char *, const char *);

  void *lib = dlopen("./" MODULE, RTLD_LAZY);
  if (!lib) {
    return 1;
  }

  dlerror();
  msg_t show_message = (msg_t) dlsym(lib, "simple_message");

  if (dlerror()) {
    dlclose(lib);
    return 1;
  }

  show_message("Error", "An error has occured!");
  dlclose(lib);

  return 0;
}
