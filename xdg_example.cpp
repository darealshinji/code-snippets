// using SimpleIni to get XDG_DESKTOP_DIR value
// https://github.com/brofield/simpleini
#include <iostream>
#include <stdlib.h>
#include "SimpleIni.h"


int main()
{
  char *env = getenv("HOME");

  if (!env || *env == 0) {
    std::cerr << "getenv() failed to get $HOME" << std::endl;
    return 1;
  }

  std::string home = env;
  if (home.back() == '/') home.pop_back();

  std::string conf = home + "/.config/user-dirs.dirs";

  CSimpleIniA ini;
  ini.SetMultiKey(false);
  ini.SetMultiLine(false);
  ini.SetQuotes(true);

  if (ini.LoadFile(conf.c_str()) < 0) {
    std::cerr << "failed to load file: " << conf << std::endl;
    return 1;
  }

  const char *key = ini.GetValue("", "XDG_DESKTOP_DIR");

  if (!key || *key == 0) {
    std::cerr << "failed to retrieve value for XDG_DESKTOP_DIR" << std::endl;
    return 1;
  }

  if (strcmp(key, "$HOME/") == 0 || strcmp(key, "$HOME") == 0) {
    return 1;
  }

  if (strncmp(key, "$HOME/", 6) == 0) {
    std::cout << home << key + 5 << std::endl;
  } else {
    std::cout << key << std::endl;
  }

  return 0;
}
