// Using dlopen() to check if libraries are present and can be used.

// gcc -m32 -Wall -O3 -DARCH='"i386"' -o checklibs32 checklibs.c -s -ldl
// gcc -m64 -Wall -O3 -DARCH='"x86_64"' -o checklibs64 checklibs.c -s -ldl

#include <dlfcn.h>
#include <stdio.h>

int checklib(const char *lib)
{
  int rv = 0;
  void *handle = dlopen(lib, RTLD_LAZY);

  if (handle)
  {
    printf("FOUND   %s\n", lib);
    dlclose(handle);
  }
  else
  {
    printf("MISSING %s\n", lib);
    rv = 1;
  }

  return rv;
}

int main(int argc, char *argv[])
{
  int rv = 0;

  if (argc == 1)
  {
    printf("usage: %s lib [lib...]\n", argv[0]);
  }
  else
  {
#ifdef ARCH
    printf("Architecture: " ARCH "\n");
#endif

    for (int i = 1; i < argc; ++i)
    {
      if (checklib(argv[i]) == 1)
      {
        rv = 1;
      }
    }
  }

  return rv;
}
