#ifdef _WIN32
  #include <windows.h>
#else
  #include <link.h>
  #include <dlfcn.h>
#endif
#include <cassert>
#include <string>
#include <vector>

// helper macros for typedefs, function declaration and
// casting function from pointer
#define LL_DEF(TYPE,NAME,PARAM)        typedef TYPE (*NAME##_t_) PARAM;
#define LL_DEC(NAME,SUFFIX)            NAME##_t_ NAME##SUFFIX = nullptr;
#define LL_CAST(NAME,SUFFIX,POINTER)   NAME##SUFFIX = reinterpret_cast<NAME##_t_>(POINTER);

// do typedef, function declaration and pointer casting at once
#define LL_FUNC(TYPE, NAME,SUFFIX, PARAM, POINTER) \
  typedef TYPE (*NAME##_LL_t) PARAM; \
  NAME##_LL_t NAME##SUFFIX = reinterpret_cast<NAME##_LL_t>(POINTER);

#define LL_STRINGIFY(x) #x

// same as LL_FUNC() makro but it also creates strings
// for TYPE, NAME, SUFFIX and PARAM
#define LL_FUNC2(TYPE, NAME,SUFFIX, PARAM, POINTER) \
  LL_FUNC(TYPE, NAME,SUFFIX, PARAM, POINTER) \
  const char *NAME##SUFFIX##_return_type = LL_STRINGIFY(TYPE); \
  const char *NAME##SUFFIX##_function_name = LL_STRINGIFY(NAME); \
  const char *NAME##SUFFIX##_suffix = LL_STRINGIFY(SUFFIX); \
  const char *NAME##SUFFIX##_parameters = LL_STRINGIFY(PARAM); \
  const char *NAME##SUFFIX##_str = LL_STRINGIFY(TYPE) " " LL_STRINGIFY(NAME) LL_STRINGIFY(SUFFIX) LL_STRINGIFY(PARAM);


// helper class to ease up dynamic library loading on Windows and POSIX systems
class Load_library
{
private:

#ifdef _WIN32
  enum { DEFAULT_FLAGS = 0 };
  HMODULE handle_;
  std::wstring wpath_;
#else
  typedef void * FARPROC;
  enum { DEFAULT_FLAGS = RTLD_LAZY };
  void *handle_;
#endif

  FARPROC last_symbol_;
  std::string path_;

  typedef struct {
    std::string name;
    FARPROC address;
  } sym_t;

  std::vector<sym_t> symbols_;

public:

  // c'tor does nothing
  Load_library() : handle_(NULL), last_symbol_(NULL)
  {}

  // c'tor loads library
  Load_library(const char *filename, int flags=DEFAULT_FLAGS)
  : handle_(NULL), last_symbol_(NULL)
  {
    load(filename, flags);
  }

#ifdef _WIN32
  // c'tor loads library (wide character version)
  Load_library(const wchar_t *wfilename, int flags=DEFAULT_FLAGS)
  : handle_(NULL), last_symbol_(NULL)
  {
    load(wfilename, flags);
  }
#endif

  // d'tor
  ~Load_library() {
    free_library();
  }

  // load library
  bool load(const char *filename, int flags=DEFAULT_FLAGS)
  {
    if (!free_library()) return false;
#ifdef _WIN32
    handle_ = ::LoadLibraryExA(filename, NULL, flags);
#else
    handle_ = ::dlopen(filename, flags);
#endif
    return (handle_ != NULL);
  }

#ifdef _WIN32
  // load library (wide character version)
  bool load(const wchar_t *wfilename, int flags=DEFAULT_FLAGS)
  {
    if (!free_library()) return false;
    handle_ = ::LoadLibraryExW(wfilename, NULL, flags);
    return (handle_ != NULL);
  }
#endif

  // load symbol
  bool load_symbol(const char *symbol)
  {
    if (!symbol || *symbol == 0) return false;

    FARPROC p;
#ifdef _WIN32
    p = ::GetProcAddress(handle_, symbol);
#else
    p = ::dlsym(handle_, symbol);
#endif

    if (!p) return false;
    last_symbol_ = p;

    for (auto e : symbols_) {
      // replace pointer if symbol already exists
      if (e.name.compare(symbol) == 0) {
        e.address = last_symbol_;
        return true;
      }
    }
    symbols_.push_back( {symbol, last_symbol_} );

    return true;
  }

  // free/release/close library
  bool free_library()
  {
    if (!handle_) return true;

#ifdef _WIN32
    // MSDN: If the function succeeds, the return value is nonzero.
    // If the function fails, the return value is zero.
    if (::FreeLibrary(handle_) == 0)
#else
    // man dlclose: On success, dlclose() returns 0; on error, it returns a nonzero value.
    if (::dlclose(handle_) != 0)
#endif
    {
      return false;
    }

    symbols_.clear();
    last_symbol_ = NULL;
    handle_ = NULL;

    return true;
  }

  // return symbol address by name
  FARPROC get_symbol(const char *symbol) const
  {
    if (!symbol || *symbol == 0) return NULL;

    for (auto e : symbols_) {
      if (e.name.compare(symbol) == 0) return e.address;
    }
    return NULL;
  }

  // return pointer to the last successfully loaded symbol;
  // value is NULL if no symbols were loaded yet
  FARPROC last_symbol() const {return last_symbol_;}

  // return true if library was successfully loaded
  bool is_loaded() const {return (handle_ != NULL);}

  // return the library's file path
  const char *path()
  {
    if (!path_.empty()) return path_.c_str();

#ifdef _WIN32
    char buf[2048];

    if (::GetModuleFileNameA(handle_, reinterpret_cast<char *>(&buf), sizeof(buf)-1) == 0 ||
        ::GetLastError() != ERROR_SUCCESS)
    {
      return NULL;
    }
    path_ = buf;
#else
    const struct link_map *lm = 0;

    if (::dlinfo(handle_, RTLD_DI_LINKMAP, &lm) != 0 || lm->l_name == NULL || *lm->l_name == 0) {
      return NULL;
    }
    path_ = lm->l_name;
#endif

    return path_.c_str();
  }

#ifdef _WIN32
  // return the library's file path (wide character version)
  const wchar_t *wpath()
  {
    wchar_t buf[2048];

    if (!wpath_.empty()) return wpath_.c_str();

    if (::GetModuleFileNameW(handle_, reinterpret_cast<wchar_t *>(&buf), sizeof(buf)-1) == 0 ||
        ::GetLastError() != ERROR_SUCCESS)
    {
      return NULL;
    }

    wpath_ = buf;
    return wpath_.c_str();
  }
#endif

};



// template class to automate the cast from a function address
// pointer to a usable function, as an alternative to a macro;
// pro: asserts if casted from null pointer
// con: function must be called with exec() method
template <typename T=void, typename... TParameters>
class Function_from_pointer
{
  typedef T (*functype_t_) (TParameters...);
  functype_t_ exec_;

public:

#ifdef _WIN32
  Function_from_pointer(FARPROC ptr)
#else
  Function_from_pointer(void *ptr)
#endif
  {
    assert(ptr != NULL);
    exec_ = reinterpret_cast<functype_t_>(ptr);
  }

  T exec(TParameters... param)
  {
    return exec_(param...);
  }

};
