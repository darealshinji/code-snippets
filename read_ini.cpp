#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <string.h>
#include <strings.h>

int load_ini(const char *file, std::vector<std::string> &vec, unsigned int max_size)
{
  std::string line;
  std::ifstream ifs(file, std::ios::in|std::ios::ate);

  const unsigned int def_min_size = 128;
  const unsigned int def_max_size = 1024*1024;

  if (!ifs.is_open()) {
    return 1;
  }

  if (max_size < def_min_size) {
    max_size = def_max_size;
  }

  if (ifs.tellg() > max_size) {
    ifs.close();
    return -1;
  }

  ifs.seekg(0, std::ios::beg);

  while (std::getline(ifs, line)) {
    if (line.front() == ';' || line.front() == '#') {
      /* commentary */
      continue;
    }

    if (line.back() == '\r') {
      /* remove carriage return */
      line.pop_back();
    }

    if (line.size() == 0) {
      /* empty line */
      continue;
    }

    /* section? */
    if (line.front() == '[') {
      /* remove trailing spaces and tabs */
      while (line.back() == ' ' || line.back() == '\t') {
        line.pop_back();
      }

      /* invalid line: missing trailing bracket */
      if (line.back() != ']') {
        continue;
      }

      /* remove trailing bracket */
      line.pop_back();
    }

    vec.push_back(line);
  }

  return 0;
}

std::string get_ini_val(std::vector<std::string> &vec, const char *want_section, const char *key, std::string default_value)
{
  std::string line;
  bool section_found;
  size_t key_size;

  if (vec.size() == 0 || !key || strlen(key) == 0) {
    return default_value;
  }

  key_size = strlen(key);

  if (want_section && strlen(want_section) == 0) {
    want_section = NULL;
  }

  section_found = (want_section == NULL);

  for (size_t i = 0; i < vec.size(); ++i) {
    if (!section_found && vec.at(i).front() != '[') {
      /* skip until we found a section entry */
      continue;
    }

    /* found a section */
    if (vec.at(i).front() == '[') {
      if (section_found || !want_section) {
        /* we don't want a section or we have already found
         * our section and a new one begins here */
        break;
      }
      section_found = (strcasecmp(want_section, vec.at(i).substr(1).c_str()) == 0);
      continue;
    }

    if (section_found && vec.at(i).size() > key_size && vec.at(i)[key_size] == '=' &&
        strncasecmp(key, vec.at(i).c_str(), key_size) == 0)
    {
      /* key found */
      return vec.at(i).substr(key_size + 1);
    }
  }

  return default_value;
}

int main(void)
{
  std::string val_w, val_h, val_defw, val_defh;
  std::filebuf fb;
  std::vector<std::string> vec;
  const char *ini = "test.ini";

  int loaded = load_ini(ini, vec, 0);

  if (loaded > 0) {
    std::cerr << "warning: cannot read file `" << ini << "'" << std::endl;
  } else if (loaded < 0) {
    std::cerr << "warning: `" << ini << "' exceeds maximum file size" << std::endl;
  }

  val_w = get_ini_val(vec, NULL, "width", "1280");
  val_h = get_ini_val(vec, NULL, "height", "720");
  val_defw = get_ini_val(vec, "Default", "width", "1920");
  val_defh = get_ini_val(vec, "Default", "height", "1080");

  vec.clear();

  fb.open(ini, std::ios::out);
  if (!fb.is_open()) {
    std::cerr << "error: cannot write into file `" << ini << "'" << std::endl;
    return 1;
  }

  std::string str = "; ini example"
    "\nwidth=" + val_w +
    "\nheight=" + val_h +
    "\n\n[Default]"
    "\nwidth=" + val_defw +
    "\nheight=" + val_defh +
    "\n";

  std::ostream os(&fb);
  os << str;
  fb.close();

  return 0;
}
