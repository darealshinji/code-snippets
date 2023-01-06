#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif


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
  if (fp) fclose(fp);
}

/**
 * file: .ass input
 * dir: attachment output directory; empty == current dir
 * extract: whether to extract attachments
 * stripped: stripped .ass output; empty == disabled
 */
bool ass_extract(const std::string &file, std::string dir, bool extract, const std::string &stripped)
{
  std::string line;
  std::fstream ifs, ofs;
  FILE *fp = NULL;
  size_t len;

  if (!extract) {
    if (stripped.empty()) return true; /* nothing to do */
    dir.clear();
  }

  ifs.open(file.c_str(), std::fstream::in);

  if (!ifs.is_open()) {
    std::cerr << "error: cannot open file for reading: "
      << file << std::endl;
    return false;
  }

  if (!stripped.empty() && stripped != file) {
    ofs.open(stripped.c_str(), std::fstream::out|std::fstream::binary);

    if (!ofs.is_open()) {
      std::cerr << "error: cannot open file for writing: "
        << stripped << std::endl;
      return false;
    }
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

  if (ofs.is_open()) ofs << line << "\r\n";

  /* seek for [Fonts] or [Graphics] group */
  while (std::getline(ifs, line)) {
    if (line.back() == '\r') line.pop_back();
    if (ofs.is_open()) ofs << line << "\r\n";
    if (line == "[Events]") break;
    if (line == "[Fonts]" || line == "[Graphics]") goto JMP_DEC;
    if (line.empty()) continue;
  }

  /* didn't find [Fonts] or [Graphics] */
  if (ofs.is_open()) {
    ofs << line << "\r\n";

    while (std::getline(ifs, line)) {
      if (line.back() == '\r') line.pop_back();
      ofs << line << "\r\n";
    }

    return true;
  } else {
    return (line == "[Events]") ? true : false;
  }

JMP_DEC:

#ifdef _WIN32
  if (!dir.empty()) {
    _mkdir(dir.c_str());
    if (dir.back() != '/' && dir.back() != '\\') dir += '\\';
  }
#else
  if (!dir.empty() && dir != "/") {
    mkdir(dir.c_str(), 00775);
    if (dir.back() != '/') dir += '/';
  }
#endif

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

      line.insert(0, dir);

      if (extract && (fp = fopen(line.c_str(), "wb")) == NULL)
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

  ass_uudecode_flush(fp);

  if (ofs.is_open()) {
    while (std::getline(ifs, line)) {
      if (line.back() == '\r') line.pop_back();
      ofs << line << "\r\n";
    }
  }

  return true;
}

int main()
{
  if (ass_extract("test.ass", "out", true, "test-stripped.ass") == true) {
    return 0;
  }
  return 1;
}

