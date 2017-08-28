/*
 gcc -Wall -O2 -shared -fPIC -o svg_to_png_librsvg.so svg_to_png_librsvg_lib.c \
   -s $(pkg-config --cflags --libs librsvg-2.0) && \
 xxd -i svg_to_png_librsvg.so > svg_to_png_librsvg_so.h && \
 g++ -Wall -O2 -o svg_to_png_librsvg svg_to_png_librsvg_main.cpp -s -ldl
*/

#include <fstream>
#include <iostream>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "svg_to_png_librsvg_so.h"
#define LIBRARY_DATA     svg_to_png_librsvg_so
#define LIBRARY_LENGTH   svg_to_png_librsvg_so_len
#define TEMPLATE_STRING  "/tmp/svg_to_png_librsvg.so-XXXXXX"
#define SYMBOL_NAME      svg_to_png_librsvg
#define SYMBOL_STRING    "svg_to_png_librsvg"

int svg_to_png(const char *in, const char *out)
{
  const char *template_string = TEMPLATE_STRING;
  size_t template_length = strlen(template_string) + 1;
  char filename[template_length];
  int (*SYMBOL_NAME) (const char*, const char*);
  void *handle;
  const char *dlsym_error;
  int rv = 1;

  /* save embedded library to temporary file */

  snprintf(filename, template_length, "%s", template_string);

  if (mkstemp(filename) == -1)
  {
    std::cerr << "Error: cannot create temporary file: " << filename << std::endl;
    return 1;
  }

  std::ofstream outfile(filename, std::ios::out|std::ios::binary);
  if(!outfile)
  {
    std::cerr << "Error: cannot open file: " << filename << std::endl;
    return 1;
  }

  outfile.write((char *)LIBRARY_DATA, (std::streamsize)LIBRARY_LENGTH);
  outfile.close();

  /* dlopen() temporary file; I find this easier than dlopen()ing all needed
   * symbols from librsvg, libcairo and libgio */

  handle = dlopen(filename, RTLD_LAZY);
  dlsym_error = dlerror();
  if (!handle)
  {
    std::cerr << dlsym_error << std::endl;
    return 1;
  }
  dlerror();

  *(void **) (&SYMBOL_NAME) = dlsym(handle, SYMBOL_STRING);
  dlsym_error = dlerror();
  if (dlsym_error)
  {
    std::cerr << "Error: cannot load symbol" << dlsym_error << std::endl;
    dlclose(handle);
    return 1;
  }

  rv = SYMBOL_NAME(in, out);

  if (dlclose(handle) != 0)
  {
    std::cerr << dlsym_error << std::endl;
    return 1;
  }
  unlink(filename);

  return rv;
}

int main(void)
{
  const char *in = "fltk/Applications-multimedia.svg";
  const char *out = "Applications-multimedia.png";
  return svg_to_png(in, out);
}

