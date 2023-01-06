#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>


static std::string src;

void ass_uudecode(const std::string &data, FILE *fp, bool flush)
{
  unsigned char dst[3], cpy[4];

  if (!fp) return;
  if (!flush) src += data;

  while (src.size() >= sizeof(cpy)) {
    dst[0] = ((src.at(0) - 33) << 2) | ((src.at(1) - 33) >> 4);
    dst[1] = ((src.at(1) - 33) << 4) | ((src.at(2) - 33) >> 2);
    dst[2] = ((src.at(2) - 33) << 6) |  (src.at(3) - 33);
    fwrite(dst, 1, sizeof(dst), fp);
    src.erase(0, sizeof(cpy));
  }

  if (!flush) return;

  /* flush remains of the src buffer */

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

  src.clear();
}

inline void ass_uudecode_flush(FILE *fp) {
  ass_uudecode("", fp, true);
  fclose(fp);
}

bool ass_extract(const std::string &file)
{
  std::string line;
  std::fstream ifs, ofs;
  FILE *fp = NULL;
  size_t len;

  ifs.open(file.c_str(), std::fstream::in);

  if (!ifs.is_open()) {
    std::cerr << "error: cannot open file for reading: "
      << file << std::endl;
    return false;
  }

  /* check first line */

  if (!std::getline(ifs, line)) {
    std::cerr << "error: cannot read first line from file: "
      << file << std::endl;
    return false;
  }

  if (line.back() == '\r') line.pop_back();

  if (line != "[Script Info]" && 
      line != "\xEF\xBB\xBF[Script Info]")
  {
    std::cerr << "error: file has incorrect first line: "
      << file << std::endl;
    return false;
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

    if (line.empty() ||
        line == "[Fonts]" ||
        line == "[Graphics]")
    {
      if (fp) {
        ass_uudecode_flush(fp);
        fp = NULL;
      }
      continue;
    }

    /* filename */
    if (line.compare(0, 10, "fontname: ") == 0 ||
        line.compare(0, 10, "filename: ") == 0)
    {
      if (fp) ass_uudecode_flush(fp);
      line.erase(0, 10);

      /* rename .ttf file */
      if ((len = line.size()) > 6 &&
          (line.compare(len-6, 6, "_0.ttf") == 0 ||
           line.compare(len-6, 6, "_0.TTF") == 0))
      {
        line.erase(len-6, 2);
      }

      if ((fp = fopen(line.c_str(), "wb")) == NULL)
      {
        std::cerr << "error: cannot open output file: "
          << line << std::endl;
        return false;
      }

      std::cout << line << std::endl;
      continue;
    }

    ass_uudecode(line, fp, false);
  }

  if (fp) ass_uudecode_flush(fp);

  return true;
}

int main()
{
  if (ass_extract("test.ass") == true) {
    return 0;
  }
  return 1;
}

