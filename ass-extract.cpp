#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>


void ass_uudecode(std::string &src, FILE *fp)
{
  unsigned char dst[3], cpy[4];

  if (!fp) return;

  while (src.size() >= sizeof(cpy)) {
    dst[0] = ((src.at(0) - 33) << 2) | ((src.at(1) - 33) >> 4);
    dst[1] = ((src.at(1) - 33) << 4) | ((src.at(2) - 33) >> 2);
    dst[2] = ((src.at(2) - 33) << 6) |  (src.at(3) - 33);
    fwrite(dst, 1, sizeof(dst), fp);
    src.erase(0, sizeof(cpy));
  }

  memset(cpy, 33, sizeof(cpy));

  switch (src.size()) {
    case 3:
      cpy[2] = src.at(2);
    case 2:
      cpy[1] = src.at(1);
    case 1:
      cpy[0] = src.at(0);
      break;
    default:
      return;
  }

  dst[0] = ((cpy[0] - 33) << 2) | ((cpy[1] - 33) >> 4);
  dst[1] = ((cpy[1] - 33) << 4) | ((cpy[2] - 33) >> 2);
  dst[2] = ((cpy[2] - 33) << 6) |  (cpy[3] - 33);
  fwrite(dst, 1, src.size()-1, fp);
}

inline void err(const std::string &s1, const std::string &s2) {
  std::cerr << "error: " << s1 << s2 << std::endl;
  std::exit(1);
}

bool ass_extract(const std::string &file)
{
  std::string line;
  std::fstream ifs, ofs;
  FILE *fp = NULL;
  size_t len;

  ifs.open(file.c_str(), std::fstream::in);
  if (!ifs.is_open()) err("cannot open file for reading: ", file);

  /* check first line */
  if (!std::getline(ifs, line)) err("cannot read first line from file: ", file);
  if (line.back() == '\r') line.pop_back();
  if (line != "[Script Info]" && line != "\xEF\xBB\xBF[Script Info]") {
    err("file has incorrect first line: ", file);
  }

  /* seek for [Fonts] or [Graphics] group */
  while (std::getline(ifs, line)) {
    if (line.back() == '\r') line.pop_back();
    if (line == "[Events]") return true;
    if (line == "[Fonts]" || line == "[Graphics]") goto JMP_DEC;
    if (line.empty()) continue;
  }

  /* didn't find [Fonts], [Graphics] or [Events] */
  return false;

JMP_DEC:

  /* decode files */
  while (std::getline(ifs, line)) {
    if (line.back() == '\r') line.pop_back();
    if (line == "[Events]") break;

    if (line.empty() || line == "[Fonts]" ||
        line == "[Graphics]")
    {
      if (fp) {
        fclose(fp);
        fp = NULL;
      }
      continue;
    }

    if (line.compare(0, 10, "fontname: ") == 0 ||
        line.compare(0, 10, "filename: ") == 0)
    {
      if (fp) {
        fclose(fp);
        fp = NULL;
      }

      line.erase(0, 10);

      if ((len = line.size()) > 6 &&
          (line.compare(len-6, 6, "_0.ttf") == 0 ||
           line.compare(len-6, 6, "_0.TTF") == 0))
      {
        line.erase(len-6, 2);
      }

      fp = fopen(line.c_str(), "wb");
      if (!fp) err("cannot open output file: ", line);
      std::cout << line << std::endl;
      continue;
    }

    ass_uudecode(line, fp);
  }

  if (fp) fclose(fp);

  return true;
}

int main()
{
  if (ass_extract("test.ass") == true) {
    return 0;
  }
  return 1;
}

