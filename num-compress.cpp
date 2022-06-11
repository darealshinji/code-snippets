#include <iostream>
#include <vector>

// compress a chain of ASCII digits to unsigned chars

int main()
{
  std::string str = "1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421";
  std::string tmp;
  std::vector<unsigned char>vec;
  size_t count = 0;

  for (auto it = str.begin(); it != str.end(); it++) {
    tmp.push_back(*it);

    if (atol(tmp.c_str()) > 255) {
      tmp.pop_back();
      long n = atol(tmp.c_str());
      vec.push_back(static_cast<char>(n));
      count += tmp.size();

      if (*it == '0') {
        vec.push_back(0x00);
        tmp = "";
        count++;
      } else {
        tmp = *it;
      }
    }
  }

  if (count < str.size()) {
    long n = atol(str.c_str() + count);
    vec.push_back(static_cast<char>(n));
  }

  for (auto c : vec) {
    putchar(c);
  }

  return 0;
}
