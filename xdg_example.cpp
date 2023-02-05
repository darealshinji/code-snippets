// using SimpleIni to get XDG_*_DIR values
// https://github.com/brofield/simpleini
#include <iostream>
#include <vector>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include "SimpleIni.h"


class xdg
{
private:

  struct dirs {
    std::string key;
    std::string path;
    std::string base;
  };

  std::string m_home;
  std::vector<struct dirs> m_dirs;

  void get_home()
  {
    if (!m_home.empty()) return;

    char *env = getenv("HOME");

    if (env && *env) {
      m_home = env;
      return;
    }

    /* fallback */
    struct passwd *pw = getpwuid(getuid());

    if (pw && pw->pw_dir && *pw->pw_dir) {
      m_home = pw->pw_dir;
    }

    assert(m_home.empty() == false);
  }

public:

  xdg() {
    get_home();
  }

  ~xdg() {}

  void clear() {
    m_dirs.clear();
  }

  bool load()
  {
    if (!m_dirs.empty()) return true;

    CSimpleIniA m_ini;
    m_ini.SetMultiKey(false);
    m_ini.SetMultiLine(false);
    m_ini.SetQuotes(true);

    char *env = getenv("XDG_CONFIG_HOME");
    std::string conf;

    if (env && *env) {
      conf = env;
      conf += "/user-dirs.dirs";
    } else {
      conf = m_home + "/.config/user-dirs.dirs";
    }

    if (m_ini.LoadFile(conf.c_str()) < 0) {
      return false;
    }

    std::vector<struct dirs> vec;
    CSimpleIniA::TNamesDepend keys;
    m_ini.GetAllKeys("", keys);

    for (auto it = keys.begin(); it != keys.end(); ++it) {
      struct dirs d;

      /* key */
      d.key = it->pItem;

      if (d.key.size() < 9 ||
          d.key.compare(0, 4, "XDG_") != 0 ||
          d.key.compare(d.key.size() - 4, 4, "_DIR") != 0)
      {
        continue;
      }

      /* value */
      const char *val = m_ini.GetValue("", d.key.c_str());

      if (!val || *val == 0 || strcmp(val, "$HOME/") == 0 || strcmp(val, "$HOME") == 0) {
        continue;
      }

      d.path = val;

      /* replace $HOME */
      if (d.path.compare(0, 6, "$HOME/") == 0) {
        d.path.replace(0, 5, m_home);
      }

      while (d.path.back() == '/') {
        d.path.pop_back();
      }

      /* basename */
      size_t pos = d.path.rfind('/');
      if (pos == std::string::npos) continue;
      d.base = d.path.substr(pos+1);

      m_dirs.push_back(d);
    }

    /* XDG_DESKTOP_DIR fallback */
    bool has_desktop = false;

    for (const auto &e : m_dirs) {
      if (e.key == "XDG_DESKTOP_DIR") {
        has_desktop = true;
        break;
      }
    }

    if (!has_desktop) {
      struct dirs d;
      d.key = "XDG_DESKTOP_DIR";
      d.path = m_home + "/Desktop";
      d.base = "Desktop";
      m_dirs.push_back(d);
    }

    /* sort by basename */
    auto comp = [] (const struct dirs &a, const struct dirs &b) -> bool {
      return strcasecmp(a.base.c_str(), b.base.c_str()) < 0;
    };

    std::sort(m_dirs.begin(), m_dirs.end(), comp);

    return true;
  }

  void print_all()
  {
    for (const auto &e : m_dirs) {
      std::cout << e.key << ": " << e.base << " -> " << e.path << std::endl;
    }
  }

  bool get_path(const std::string &key, std::string &path, std::string &base)
  {
    if (key.empty()) return false;

    for (const auto &e : m_dirs) {
      if (e.key == key) {
        path = e.path;
        base = e.base;
        return true;
      }
    }

    return false;
  }
};


int main()
{
  std::string p, b;
  xdg paths;

  if (!paths.load()) return 1;
  paths.print_all();

  if (paths.get_path("XDG_DESKTOP_DIR", p, b)) {
    std::cout << '\n' << b << " -> " << p << std::endl;
  }

  return 0;
}
