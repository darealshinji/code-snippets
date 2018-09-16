#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <strings.h>

std::string read_ini(std::ifstream &ifs, std::string want_section, std::string key, std::string default_value)
{
  std::string line;
  bool section_found;
  size_t key_size;

  if (!ifs.is_open()) {
    return default_value;
  }

  if (want_section.empty()) {
    want_section = "";
  }

  section_found = (want_section == "");

  key += "=";
  key_size = key.size();

  ifs.seekg(0, std::ios::beg);

  while (std::getline(ifs, line)) {
    if (!section_found && line.front() != '[') {
      /* skip until we found a section entry */
      continue;
    }

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

    if (line.front() == '[') {
      /* remove trailing spaces and tabs */
      while (line.back() == ' ' || line.back() == '\t') {
        line.pop_back();
      }

      if (line.back() != ']') {
        continue;
      }

      /* found a section */
      if (want_section == "" || section_found) {
        /* we don't want a section or we have already found
         * our section and a new one begins here */
        break;
      }

      /* remove brackets */
      line.pop_back();
      line.erase(0, 1);

      section_found = (strcasecmp(want_section.c_str(), line.c_str()) == 0);
      continue;
    }

    if (section_found && strncasecmp(key.c_str(), line.c_str(), key_size) == 0) {
      /* key found */
      return line.substr(key_size);
    }
  }

  return default_value;
}

int main(void)
{
  std::string val_w, val_h, val_defw, val_defh;
  std::filebuf fb;

  std::ifstream ifs("test.ini", std::ios::in);
  if (!ifs.is_open()) {
    return 1;
  }

  // perform filesize check?

  val_w = read_ini(ifs, "", "width", "1280");
  val_h = read_ini(ifs, "", "height", "720");
  val_defw = read_ini(ifs, "Default", "width", "1920");
  val_defh = read_ini(ifs, "Default", "height", "1080");

  ifs.close();
  fb.open("test.ini", std::ios::out);

  if (fb.is_open()) {
    std::ostream os(&fb);
    os << "; ini example"
      << "\nwidth=" << val_w
      << "\nheight=" << val_h
      << "\n\n[Default]"
      << "\nwidth=" << val_defw
      << "\nheight=" << val_defh
      << "\n";
    fb.close();
    return 0;
  }

  return 1;
}
