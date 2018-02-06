#include <dlfcn.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#define SDL_INIT_VIDEO 32
#define SDL_MESSAGEBOX_ERROR 16
typedef uint32_t Uint32;
typedef struct SDL_Window SDL_Window;

#define STRINGIFY(x) #x
#define GETPROCADDR(handle,type,func,param) \
  typedef type (*func##_t) param; \
  func##_t func = (func##_t) dlsym(handle, STRINGIFY(func));

void print_error(const char *message)
{
  void *handle = dlopen("libSDL2-2.0.so.0", RTLD_LAZY);

  if (!handle) {
    goto print;
  }

  dlerror();

#define DLSYM(type,func,param) \
  GETPROCADDR(handle,type,func,param) \
  if (dlerror()) { goto close; }

  DLSYM( int, SDL_Init, (Uint32) )
  DLSYM( int, SDL_ShowSimpleMessageBox, (Uint32,const char*,const char*,SDL_Window*) )
  DLSYM( void, SDL_Quit, (void) )

  if (SDL_Init(SDL_INIT_VIDEO) == 0) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", message, NULL);
    SDL_Quit();
  }

close:
  dlclose(handle);
print:
  fprintf(stderr, "Error: %s\n", message);
}

int main(void)
{
  print_error("This is an error message!");
  return 0;
}

