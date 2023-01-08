#include <iostream>
#include <fstream>
#include <string.h>


/* small wrapper class to enable reading input from
 * a file or STDIN using the same object */
class my_ifstream
{
private:
  bool m_stdin = false;
  std::ifstream m_ifs;

public:

  /* input is "-" -> read from std::cin */
  my_ifstream(const char *file) {
    if (strcmp(file, "-") == 0) {
      m_stdin = true;
    } else {
      m_ifs.open(file);
    }
  }

  ~my_ifstream() {
    if (m_ifs.is_open()) m_ifs.close();
  }

  bool is_open() { return m_stdin ? true : m_ifs.is_open(); }
  int get() { return m_stdin ? std::cin.get() : m_ifs.get(); }
  int peek() { return m_stdin ? std::cin.peek() : m_ifs.peek(); }
  /* ... */
};
