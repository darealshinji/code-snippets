/**
 * This is an attempt to provide the POSIX functions dlopen(), dlclose(),
 * dlsym() and dlerror() for Windows, with their behavior as close to the
 * original ones as possible.
 */

#define IMPLEMENT_DLOPEN 1
#define EXAMPLE 1




#ifdef _WIN32

#include <Windows.h>

/**
 * The LoadLibraryEx() flags are totally different from the dlopen() flags.
 * Defining the dlopen() flags all as 0 means they do nothing.
 */
#define RTLD_LAZY 0
#define RTLD_NOW 0
#define RTLD_BINDING_MASK 0
#define RTLD_NOLOAD 0
#define RTLD_DEEPBIND 0
#define RTLD_GLOBAL 0
#define RTLD_LOCAL 0
#define RTLD_NODELETE 0

/**
 * Wrapper around LoadLibraryEx().
 * Flags will be passed directly to LoadLibraryEx().
 */
void *dlopen(const char *filename, int flags);

/**
 * Wrapper around FreeLibrary().
 * Return value is that of dlclose(): 0 on success, 1 (non-zero) on error.
 */
int dlclose(void *handle);

/* Wrapper around GetProcAddress(). */
void *dlsym(void *handle, const char *symbol);

/* Returns a message created with FormatMessage() on error. */
char *dlerror(void);




#ifdef IMPLEMENT_DLOPEN

#include <stdio.h>

static char _dlerror_msg[4096];
static int _last_dl_errcode = 0;
static const char *_last_dl_function = NULL;

static void _set_dlerror_msg(void *handle, const char *filename, const char *symbol)
{
  char *err = NULL;
  const size_t sz = sizeof(_dlerror_msg);
  const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;

  memset(_dlerror_msg, '\0', sz);

  if (_last_dl_errcode == 0 || !_last_dl_function) {
    _last_dl_errcode = 0;
    _last_dl_function = NULL;
    return;
  }

  if (handle && GetModuleFileName(handle, _dlerror_msg, sizeof(_dlerror_msg)-1) != 0) {
    strncat_s(_dlerror_msg, sz, ": ", _TRUNCATE);
  } else if (filename) {
    strncat_s(_dlerror_msg, sz, filename, _TRUNCATE);
    strncat_s(_dlerror_msg, sz, ": ", _TRUNCATE);
  }

  if (symbol) {
    strncat_s(_dlerror_msg, sz, symbol, _TRUNCATE);
    strncat_s(_dlerror_msg, sz, ": ", _TRUNCATE);
  }

  if (FormatMessage(flags, NULL, _last_dl_errcode, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPTSTR)&err, 0, NULL) ||
      FormatMessage(flags, NULL, _last_dl_errcode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&err, 0, NULL))
  {
    strncat_s(_dlerror_msg, sz, err, _TRUNCATE);
    LocalFree(err);
  } else {
    char buf[128];
    _snprintf_s(buf, sizeof(buf), sizeof(buf)-1, "%s returned error code %d", _last_dl_function, _last_dl_errcode);
    strncat_s(_dlerror_msg, sz, buf, _TRUNCATE);
  }
}

void *dlopen(const char *filename, int flags)
{
  void *handle = NULL;

  if (filename) {
    handle = LoadLibraryEx(filename, NULL, flags);
    _last_dl_errcode = GetLastError();
  } else {
    _last_dl_errcode = ERROR_INVALID_PARAMETER;
  }

  _last_dl_function = "LoadLibraryEx()";
  _set_dlerror_msg(NULL, filename, NULL);

  return handle;
}

int dlclose(void *handle)
{
  if (handle) {
    _last_dl_errcode = (FreeLibrary(handle) == TRUE) ? 0 : 1;
  } else {
    _last_dl_errcode = ERROR_INVALID_PARAMETER;
  }
  _last_dl_function = "FreeLibrary()";
  _set_dlerror_msg(NULL, NULL, NULL);
  return _last_dl_errcode;
}

void *dlsym(void *handle, const char *symbol)
{
  void *p = NULL;

  if (handle && symbol) {
    p = GetProcAddress(handle, symbol);
    _last_dl_errcode = GetLastError();
  } else {
    _last_dl_errcode = ERROR_INVALID_PARAMETER;
    handle = NULL;
    symbol = NULL;
  }

  _last_dl_function = "GetProcAddress()";
  _set_dlerror_msg(handle, NULL, symbol);

  return p;
}

char *dlerror(void)
{
  _last_dl_errcode = 0;

  if (!_last_dl_function) {
    memset(_dlerror_msg, '\0', sizeof(_dlerror_msg));
    return NULL;
  }

  _last_dl_function = NULL;
  return _dlerror_msg;
}

#endif  /* IMPLEMENT_DLOPEN */

#else  /* !_WIN32 */

/* LoadLibraryEx() dummy flags for POSIX */
#define DONT_RESOLVE_DLL_REFERENCES 0
#define LOAD_IGNORE_CODE_AUTHZ_LEVEL 0
#define LOAD_LIBRARY_AS_DATAFILE 0
#define LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE 0
#define LOAD_LIBRARY_AS_IMAGE_RESOURCE 0
#define LOAD_LIBRARY_SEARCH_APPLICATION_DIR 0
#define LOAD_LIBRARY_SEARCH_DEFAULT_DIRS 0
#define LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR 0
#define LOAD_LIBRARY_SEARCH_SYSTEM32 0
#define LOAD_LIBRARY_SEARCH_USER_DIRS 0
#define LOAD_WITH_ALTERED_SEARCH_PATH 0

#endif  /* !_WIN32 */




#ifdef EXAMPLE

#ifdef _WIN32
#define ZLIB_DLL "zlib1.dll"
#else
#include <dlfcn.h>
#include <stdio.h>
#define ZLIB_DLL "libz.so.1"
#endif

int main(void)
{
  typedef const char * (*zlibVersion_t)(void);
  zlibVersion_t zlibVersion;

  void *handle;
  char *err;

  /* load library */
  handle = dlopen(ZLIB_DLL, RTLD_LAZY | LOAD_WITH_ALTERED_SEARCH_PATH);
  if (!handle) {
    printf("%s\n", dlerror());
    return 1;
  }
  printf(ZLIB_DLL " loaded\n");

  /* clear error message */
  dlerror();

  /* load symbol */
  zlibVersion = dlsym(handle, "zlibVersion");
  err = dlerror();
  if (err) {
    printf("%s\n", err);
    dlclose(handle);
    return 1;
  }

  /* use symbol */
  printf("zlibVersion() returned: %s\n", zlibVersion());

  /* close library */
  if (dlclose(handle) == 0) {
    printf(ZLIB_DLL " closed\n");
  } else {
    printf("%s\n", dlerror());
    return 1;
  }

  return 0;
}

#endif  /* EXAMPLE */


