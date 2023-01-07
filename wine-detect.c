/**
 * One way to check if an app is running with Wine is to look
 * for wine_get_version() in ntdll.dll.
 * An app compiled with winegcc can call this function directly.
 */
#include <windows.h>
#include <stdio.h>

#ifdef __WINE__
extern const char * CDECL wine_get_version();
#endif

int main()
{
  const char *fmt = "running on Wine version %s"
#ifdef __ELF__
    " (compiled as ELF binary)"
#endif
    "\n";

#ifdef __WINE__
  /* compiled with winegcc */
  printf(fmt, wine_get_version());
#else
  const char *(CDECL *fptr_wine_get_version)(void);

  HMODULE handle = GetModuleHandle("ntdll.dll");

  if (!handle) {
    puts("error: failed to load ntdll.dll");
    return 1;
  }

  fptr_wine_get_version = (void *)GetProcAddress(handle, "wine_get_version");

  if (fptr_wine_get_version) {
    printf(fmt, fptr_wine_get_version());
  } else {
    puts("Wine was not detected");
  }

  FreeLibrary(handle);
#endif

  return 0;
}
