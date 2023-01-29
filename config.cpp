/**
Simple config/ini reader class.
Format is "key=value", value can be empty, spaces are allowed, no groups.
Comments must be on separate lines beginning with ; or # (leading
spaces are okay).
The class will keep comments but delete duplicate keys and set the
value of the first found key with subsequent values of the same key.
Multiple empty lines will be replaced with a single empty line.

key1=value
key2 = value
key3=value

# overwrite key3
; with empty value
key3 =

*/
#include <iostream>
#include <iomanip>
#include <fstream>
#include <regex>
#include <string>
#include <cctype>
#include <cstring>


class conf
{
private:
  typedef struct {
    std::string key;
    std::string value;
  } pair_t;

  std::vector<pair_t> m_lines;
  bool m_ignore_errors = false;
  int m_err_line = 0;

  bool read_line(std::string &line, int i)
  {
    /* clear spaces in front and back */
    while (isspace(line.back())) line.pop_back();
    while (isspace(line.front())) line.erase(0, 1);

    if (line.empty()) {
      /* empty line */
      if (m_lines.size() > 0 && !(m_lines.back().key.empty() && m_lines.back().value.empty())) {
        m_lines.push_back({});
      }
      return true;
    } else if (line.front() == '#' || line.front() == ';') {
      /* comment */
      m_lines.push_back({"", line});
      return true;
    }

    std::smatch m;
    std::regex reg("([A-Za-z][A-Za-z0-9_]*)[?|\\s]*=[?|\\s]*(.*)");

    if (!std::regex_match(line, m, reg) || m.size() != 3) {
      if (m_ignore_errors) {
        return true;
      }
      m_err_line = i;
      return false;
    }

    /* check if key already exists and update value */
    bool found = false;

    for (auto &e : m_lines) {
      if (e.key == m[1]) {
        e.value = m[2];
        found = true;
        break;
      }
    }

    if (!found) m_lines.push_back({ m[1], m[2] });

    return true;
  }

public:
  conf() {}
  ~conf() {}

  bool load_string(const std::string &s_input)
  {
    std::string line;
    int i = 1;

    m_err_line = 0;
    clear();

    for (const auto &c : s_input) {
      if (c != '\n') {
        line.push_back(c);
        continue;
      }

      if (!read_line(line, i)) return false;

      line.clear();
      i++;
    }

    return true;
  }

  bool load_file(const std::string &file)
  {
    std::string line;
    std::fstream fs(file);

    m_err_line = 0;
    clear();

    if (!fs.is_open()) return false;

    for (int i=1; std::getline(fs, line); i++) {
      if (!read_line(line, i)) return false;
    }

    return true;
  }

  void print()
  {
    for (const auto &e : m_lines) {
      if (e.key.empty()) {
        std::cout << e.value << std::endl;
      } else {
        std::cout << e.key << " = " << e.value << std::endl;
      }
    }
  }

  size_t to_string(std::string &s)
  {
    s.clear();

    for (const auto &e : m_lines) {
      if (!e.key.empty()) {
        s += e.key + " = ";
      }
      s += e.value + "\n";
    }

    return s.size();
  }

  /* get/set string */

  bool get(const std::string &key, std::string &value)
  {
    for (const auto &e : m_lines) {
      if (e.key == key) {
        value = e.value;
        return true;
      }
    }
    return false;
  }

  void set(const std::string &key, const std::string &value)
  {
    for (auto &e : m_lines) {
      if (e.key == key) {
        e.value = value;
        return;
      }
    }
    m_lines.push_back({ key, value });
  }

  /* get/set boolean */

  bool get_bool(const std::string &key, bool &value)
  {
    std::string s;

    if (!get(key, s)) return false;

    if (s == "0" || strcasecmp(s.c_str(), "false") == 0) {
      value = false;
    } else if (s == "1" || strcasecmp(s.c_str(), "true") == 0) {
      value = true;
    } else {
      return false;
    }

    return true;
  }

  void set_bool(const std::string &key, bool value) {
    set(key, value ? "1" : "0");
  }

  /* get/set long integer */

  bool get_long(const std::string &key, long &value)
  {
    std::string s;

    if (!get(key, s) || s.empty()) return false;

    try {
      value = std::stol(s);
    }
    catch (std::invalid_argument const&) {
      return false;
    }
    catch (std::out_of_range const&) {
      return false;
    }

    return true;
  }

  void set_long(const std::string &key, long value) {
    char buf[64];
    sprintf(buf, "%ld", value);
    set(key, buf);
  }

  /* get/set float point value */

  bool get_double(const std::string &key, double &value)
  {
    std::string s;

    if (!get(key, s) || s.empty()) return false;

    try {
      value = std::stod(s);
    }
    catch (std::invalid_argument const&) {
      return false;
    }
    catch (std::out_of_range const&) {
      return false;
    }

    return true;
  }

  void set_double(const std::string &key, double value)
  {
    char buf[64];
    sprintf(buf, "%.10f", value);

    std::string s = buf;
    while (s.back() == '0') s.pop_back();
    if (s.back() == '.') s.push_back('0');

    //std::cout << "SET " << key << " TO " << s << std::endl;
    set(key, s);
  }

  /* ignore malformatted lines */
  void ignore_errors(bool b) { m_ignore_errors = b; }
  bool ignore_errors() const { return m_ignore_errors; }

  /* set to the line number if an error occured (a value greater than 0) */
  int err_line() const { return m_err_line; }

  /* delete the read data */
  void clear() { m_lines.clear(); }
};


int main()
{
  conf cfg;
  std::string file = "file.conf";

  if (cfg.load_file(file) == false) {
    std::cerr << "error loading config file \"" << file << '"' << std::endl;

    if (cfg.err_line() > 0) {
      std::cerr << "error at line " << cfg.err_line() << std::endl;
    }
    return 1;
  }

  cfg.print();

  return 0;
}
