#include <string>

/* replace_string(a,b,s) will substitute a with b in s */
void replace_string(const std::string &from, const std::string &to, std::string &s)
{
  if (!from.empty())
  {
    for (size_t pos = 0; (pos = s.find(from, pos)) != std::string::npos; pos += to.size())
    {
      s.replace(pos, from.size(), to);
    }
  }
}


#include <iostream>

using namespace std;

int main(void)
{
  string s;

  cout << "\n-- replace a word --\n" << endl;

  s = "Hello World!";
  cout << "before: " << s << endl;
  replace_string("Hello", "Goodbye", s);
  cout << "\nafter: " << s << endl;

  cout << "\n\n-- replace placeholder with newline --\n" << endl;

  s = "\\nline 1\\nline 2\\nline 3\\n";
  cout << "before: " << s << endl;
  replace_string("\\n", "\n", s);
  cout << "\nafter: " << s << endl;

  return 0;
}

