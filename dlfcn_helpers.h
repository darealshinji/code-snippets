// headers
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

// typedefs
#ifndef _WIN32
typedef void * HMODULE;
typedef void * FARPROC;
#endif
typedef HMODULE dl_handle_t;
typedef FARPROC dl_symbol_t;

// function macros
#ifdef _WIN32
#define DL_OPEN(NAME)           LoadLibraryA(NAME)
#define DL_OPENEX(NAME, FLAGS)  LoadLibraryExA(NAME, NULL, FLAGS)
#define DL_SYM(HANDLE, NAME)    GetProcAddress(HANDLE, NAME)
#define DL_CLOSE(HANDLE)        FreeLibrary(HANDLE)
#else
#define DL_OPEN(NAME)           dlopen(NAME, RTLD_LAZY)
#define DL_OPENEX(NAME, FLAGS)  dlopen(NAME, FLAGS)
#define DL_SYM(HANDLE, NAME)    dlsym(HANDLE, NAME)
#define DL_CLOSE(HANDLE)        dlclose(HANDLE)
#endif

// FreeLibrary() and dlcose() return different values
// on success, so for safety do only check against the
// DL_CLOSE_SUCCESS macro
#ifdef _WIN32
#define DL_CLOSE_SUCCESS  TRUE
#else
#define DL_CLOSE_SUCCESS  0
#endif

// function pointer typedef
#define DL_TYPEDEF(TYPE,NAME,PARAM) \
  typedef TYPE (*NAME##_t_) PARAM

// function pointer prototype declaration
#define DL_PROTO(NAME,SUFFIX) \
  NAME##_t_ NAME##SUFFIX = NULL

// cast address pointer to function pointer
#define DL_CAST(NAME,SUFFIX,POINTER) \
  NAME##SUFFIX = (NAME##_t_)POINTER

// do typedef, declaration and casting at once
#define DL_CASTFUNC(TYPE, NAME,SUFFIX, PARAM, POINTER) \
  typedef TYPE (*NAME##_t_) PARAM; \
  NAME##_t_ NAME##SUFFIX = (NAME##_t_)POINTER

// POSIX flags
#ifndef RTLD_LAZY
#define RTLD_LAZY 0
#endif
#ifndef RTLD_NOW
#define RTLD_NOW 0
#endif
#ifndef RTLD_GLOBAL
#define RTLD_GLOBAL 0
#endif
#ifndef RTLD_LOCAL
#define RTLD_LOCAL 0
#endif

// GNU extension flags
#ifndef RTLD_NODELETE
#define RTLD_NODELETE 0
#endif
#ifndef RTLD_NOLOAD
#define RTLD_NOLOAD 0
#endif
#ifndef RTLD_DEEPBIND
#define RTLD_DEEPBIND 0
#endif

// WinAPI flags
#ifndef DONT_RESOLVE_DLL_REFERENCES
#define DONT_RESOLVE_DLL_REFERENCES 0
#endif
#ifndef LOAD_IGNORE_CODE_AUTHZ_LEVEL
#define LOAD_IGNORE_CODE_AUTHZ_LEVEL 0
#endif
#ifndef LOAD_LIBRARY_AS_DATAFILE
#define LOAD_LIBRARY_AS_DATAFILE 0
#endif
#ifndef LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE
#define LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE 0
#endif
#ifndef LOAD_LIBRARY_AS_IMAGE_RESOURCE
#define LOAD_LIBRARY_AS_IMAGE_RESOURCE 0
#endif
#ifndef LOAD_LIBRARY_SEARCH_APPLICATION_DIR
#define LOAD_LIBRARY_SEARCH_APPLICATION_DIR 0
#endif
#ifndef LOAD_LIBRARY_SEARCH_DEFAULT_DIRS
#define LOAD_LIBRARY_SEARCH_DEFAULT_DIRS 0
#endif
#ifndef LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR
#define LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR 0
#endif
#ifndef LOAD_LIBRARY_SEARCH_SYSTEM32
#define LOAD_LIBRARY_SEARCH_SYSTEM32 0
#endif
#ifndef LOAD_LIBRARY_SEARCH_USER_DIRS
#define LOAD_LIBRARY_SEARCH_USER_DIRS 0
#endif
#ifndef LOAD_WITH_ALTERED_SEARCH_PATH
#define LOAD_WITH_ALTERED_SEARCH_PATH 0
#endif
#ifndef LOAD_LIBRARY_REQUIRE_SIGNED_TARGET
#define LOAD_LIBRARY_REQUIRE_SIGNED_TARGET 0
#endif
#ifndef LOAD_LIBRARY_SAFE_CURRENT_DIRS
#define LOAD_LIBRARY_SAFE_CURRENT_DIRS 0
#endif

// will default to 0 on Windows
#define DL_FLAGS_DEFAULT  RTLD_LAZY



// example code to load math library and use the "cos" function
/*
int main()
{
#ifdef _WIN32
	const char *name = "api-ms-win-crt-math-l1-1-0.dll";
#else
	const char *name = "libm.so.6";
#endif

	dl_handle_t handle = DL_OPEN(name);

	if (!handle) {
		printf("error loading %s\n", name);
		return 1;
	}

	dl_symbol_t sym = DL_SYM(handle, "cos");

	if (!sym) {
		printf("error loading symbol 'cos'\n");
		DL_CLOSE(handle);
		return 1;
	}

	DL_CASTFUNC(double, cos, _XX, (double), sym);

	printf("cos(2) == %f\n", cos_XX(2));

	if (DL_CLOSE(handle) != DL_CLOSE_SUCCESS) {
		printf("error closing library\n");
		return 1;
	}

	return 0;
}
*/
