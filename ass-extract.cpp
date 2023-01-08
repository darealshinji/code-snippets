/**
 Copyright (c) 2014, Christoph "Youka" Spanknebel
 Copyright (c) 2023, djcj <djcj@gmx.de>

 This software is provided 'as-is', without any express or implied warranty.
 In no event will the authors be held liable for any damages arising from the
 use of this software.

 Permission is granted to anyone to use this software for any purpose, including
 commercial applications, and to alter it and redistribute it freely, subject to
 the following restrictions:

 1. The origin of this software must not be misrepresented; you must not claim
    that you wrote the original software. If you use this software in a product,
    an acknowledgment in the product documentation would be appreciated but is
    not required.
 2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.
 3. This notice may not be removed or altered from any source distribution.
*/
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h> /* _mkdir() */
#else
#include <sys/stat.h> /* mkdir(2) */
#endif


namespace ass
{

enum {
  M_EXTRACT = 0,
  M_STRIP   = 1,
  M_LIST    = 2
};

std::string src_buf;


bool uuencode(const std::string &file, std::fstream &ofs)
{
  FILE *fp;
  unsigned char src[3], dst[4];
  size_t read, len, wrote_line = 0;

  if ((fp = fopen(file.c_str(), "rb")) == NULL) {
    std::cerr << "error: cannot open file for reading: "
      << file << std::endl;
    return false;
  }

  while ((read = fread(src, 1, sizeof(src), fp)) != 0) {
    memset(src+read, 0, sizeof(src)-read);

    dst[0] =                          (src[0] >> 2)  + 33;
    dst[1] = (((src[0] & 0x3) << 4) | (src[1] >> 4)) + 33;
    dst[2] = (((src[1] & 0xf) << 2) | (src[2] >> 6)) + 33;
    dst[3] =   (src[2] & 0x3f)                       + 33;

    len = std::min(read+1, sizeof(dst));
    ofs.write((char *)dst, len);
    wrote_line += len;

    if (wrote_line >= 80 && !feof(fp)){
      ofs << "\r\n";
      wrote_line = 0;
    }
  }

  if (wrote_line != 0) {
    ofs << "\r\n";
  }

  fclose(fp);
  return true;
}

void uudecode(const std::string &data, FILE *fp)
{
  unsigned char dst[3];

  if (!fp) return;

  src_buf += data;

  while (src_buf.size() >= 4) {
    dst[0] = ((src_buf.at(0) - 33) << 2) | ((src_buf.at(1) - 33) >> 4);
    dst[1] = ((src_buf.at(1) - 33) << 4) | ((src_buf.at(2) - 33) >> 2);
    dst[2] = ((src_buf.at(2) - 33) << 6) |  (src_buf.at(3) - 33);
    fwrite(dst, 1, sizeof(dst), fp);
    src_buf.erase(0, 4);
  }
}

/* flush remains of the src_buf buffer */
void uudecode_flush(FILE *fp)
{
  unsigned char src[4], dst[3];

  memset(src, 33, sizeof(src));

  switch (src_buf.size()) {
    case 3:
      src[2] = src_buf.at(2);
    case 2:
      src[1] = src_buf.at(1);
    case 1:
      src[0] = src_buf.at(0);
      break;
    default:
      return;
  }

  dst[0] = ((src[0] - 33) << 2) | ((src[1] - 33) >> 4);
  dst[1] = ((src[1] - 33) << 4) | ((src[2] - 33) >> 2);
  dst[2] = ((src[2] - 33) << 6) |  (src[3] - 33);
  fwrite(dst, 1, src_buf.size()-1, fp);

  src_buf.clear();
}

std::string check_first_line(std::fstream &ifs, const std::string &file)
{
  std::string line;

  if (!ifs.is_open()) return {};

  if (!std::getline(ifs, line)) {
    std::cerr << "error: cannot read first line from file: "
      << file << std::endl;
    return {};
  }

  if (line.back() == '\r') line.pop_back();

  if (line != "[Script Info]" && 
      line != "\xEF\xBB\xBF[Script Info]")
  {
    std::cerr << "error: file has incorrect first line: "
      << file << std::endl;
    return {};
  }

  return line;
}

static bool attach_fonts(std::fstream &ofs, const std::vector<std::string> &fonts)
{
  size_t pos, len;

  for (const auto &e : fonts) {
    std::string s = e;

#ifdef _WIN32
    if ((pos = s.find_last_of("\\/")) != std::string::npos)
#else
    if ((pos = s.rfind('/')) != std::string::npos)
#endif
    {
      s.erase(0, pos+1);
    }

    if ((len = s.size()) > 4 &&
        strcasecmp(s.c_str() + (len-4), ".TTF") == 0)
    {
      s.insert(len-4, "_0");
    } else {
      s += "_0";
    }
    ofs << "fontname: " << s << "\r\n";

    if (!uuencode(e, ofs)) {
      return false;
    }
  }

  return true;
}

/**
 * file: .ass input
 * mode:
 *   M_EXTRACT: extract all attachments; strip .ass in "stripped" is not empty
 *   M_STRIP: strip only
 *   M_LIST: list only attachments
 * dir: attachment output directory; empty == current dir
 * stripped: stripped .ass output; empty == disabled
 */
bool extract(const std::string &file, int mode, std::string dir, const std::string &stripped)
{
  std::string line;
  std::fstream ifs, ofs;
  const char *group = "";
  FILE *fp = NULL;
  size_t len;

  if (mode != M_EXTRACT) dir.clear();

  if (mode == M_STRIP && stripped.empty()) {
    return true;
  }

  ifs.open(file.c_str(), std::fstream::in);

  if (!ifs.is_open()) {
    std::cerr << "error: cannot open file for reading: "
      << file << std::endl;
    return false;
  }

  if (mode != M_LIST && !stripped.empty() && stripped != file) {
    ofs.open(stripped.c_str(), std::fstream::out);

    if (!ofs.is_open()) {
      std::cerr << "error: cannot open file for writing: "
        << stripped << std::endl;
      return false;
    }
  }

  /* check first line */
  line = check_first_line(ifs, file);
  if (line.empty()) return false;
  if (ofs.is_open()) ofs << line << "\r\n";

  /* seek for [Fonts] or [Graphics] group */
  while (std::getline(ifs, line)) {
    if (line.back() == '\r') line.pop_back();

    if (line == "[Fonts]") {
      group = "[Fonts] ";
      goto JMP_DEC;
    } else if (line == "[Graphics]") {
      group = "[Graphics] ";
      goto JMP_DEC;
    }

    if (ofs.is_open()) ofs << line << "\r\n";
    if (line == "[Events]") break;
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

    if (line == "[Events]") {
      if (ofs.is_open()) ofs << "[Events]\r\n";
      break;
    }

    if (line == "[Fonts]") {
      group = "[Fonts] ";
      line.clear(); /* forces a flush */
    } else if (line == "[Graphics]") {
      group = "[Graphics] ";
      line.clear(); /* forces a flush */
    }

    if (line.empty()) {
      if (fp) {
        uudecode_flush(fp);
        fp = NULL;
      }
      continue;
    }

    /* filename */
    if (line.compare(0, 10, "fontname: ") == 0 ||
        line.compare(0, 10, "filename: ") == 0)
    {
      if (fp) uudecode_flush(fp);
      line.erase(0, 10);

      /* rename .ttf file */
      if (mode == M_EXTRACT) {
        if ((len = line.size()) > 6 &&
            strcasecmp(line.c_str() + (len-6), "_0.TTF") == 0)
        {
          line.erase(len-6, 2);
        }

        line.insert(0, dir);

        if ((fp = fopen(line.c_str(), "wb")) == NULL) {
          std::cerr << "error: cannot open output file: "
            << line << std::endl;
          return false;
        }
      }

      if (mode != M_STRIP) {
        std::cout << group << line << std::endl;
      }
      continue;
    }

    uudecode(line, fp);
  }

  uudecode_flush(fp);

  if (ofs.is_open()) {
    while (std::getline(ifs, line)) {
      if (line.back() == '\r') line.pop_back();
      ofs << line << "\r\n";
    }

    std::cout << "file `" << stripped << "' successfully written" << std::endl;
  }

  return true;
}

bool attach(const std::string &infile,
            const std::string &outfile,
            std::vector<std::string> &vfonts,
            std::vector<std::string> &vgraphics)
{
  std::fstream ifs, ofs;
  std::string line;
  size_t pos;
  bool has_fonts = false;
  bool has_graphics = false;

  if (vfonts.empty() && vgraphics.empty()) {
    return false;
  }

  ifs.open(infile.c_str(), std::fstream::in);

  if (!ifs.is_open()) {
    std::cerr << "error: cannot open file for reading: "
      << infile << std::endl;
    return false;
  }

  ofs.open(outfile.c_str(), std::fstream::out);

  if (!ofs.is_open()) {
    std::cerr << "error: cannot open file for writing: "
      << outfile << std::endl;
    return false;
  }

  line = check_first_line(ifs, infile);
  if (line.empty()) return false;
  ofs << line << "\r\n";

  /* seek for [Fonts] or [Graphics] group */
  while (std::getline(ifs, line)) {
    if (line.back() == '\r') line.pop_back();

    if (line == "[Fonts]") {
      has_fonts = true;
      ofs << "[Fonts]\r\n";

      if (!attach_fonts(ofs, vfonts)) {
        return false;
      }
      continue;
    }
    else if (line == "[Graphics]") {
      has_graphics = true;
      ofs << line << "\r\n";

      for (const auto &e : vgraphics) {
        std::string s = e;

        if ((pos = s.rfind('/')) != std::string::npos) {
          s.erase(0, pos-1);
        }
        ofs << "filename: " << s << "\r\n";

        if (!uuencode(e, ofs)) {
          return false;
        }
      }
      continue;
    }

    if (line == "[Events]") break;
    ofs << line << "\r\n";
  }

  if (line != "[Events]") return false;

  if (!has_fonts && !vfonts.empty()) {
    ofs << "[Fonts]\r\n";

    if (!attach_fonts(ofs, vfonts)) {
      return false;
    }
    ofs << "\r\n";
  }

  if (!has_graphics && !vgraphics.empty()) {
    ofs << "[Graphics]\r\n";

    for (const auto &e : vgraphics) {
      std::string s = e;

      if ((pos = s.rfind('/')) != std::string::npos) {
        s.erase(0, pos-1);
      }
      ofs << "filename: " << s << "\r\n";

      if (!uuencode(e, ofs)) {
        return false;
      }
    }
    ofs << "\r\n";
  }

  ofs << "[Events]\r\n";

  while (std::getline(ifs, line)) {
    if (line.back() == '\r') line.pop_back();
    ofs << line << "\r\n";
  }

  std::cout << "file `" << outfile << "' successfully written" << std::endl;

  return true;
}

} /* namespace ass end */


int main(int argc, char *argv[])
{
  const char *arg = "<noarg>";
  int rv = 1;

  if (argc < 2) goto JMP_MISS;

  /* help */
  for (int i=1; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      rv = 0;
      goto JMP_HELP;
    }
  }

  /* list attachments */
  if (strcmp(argv[1], "l") == 0 || strcmp(argv[1], "list") == 0) {
    if (argc < 3) goto JMP_MISS;
    return (ass::extract(argv[2], ass::M_LIST, {}, {}) == true) ? 0 : 1;
  }

  /* extract attachments */
  if (strcmp(argv[1], "x") == 0 || strcmp(argv[1], "extract") == 0) {
    const char *dir = "";
    const char *stripped = "";

    if (argc < 3) {
      goto JMP_MISS;
    } else if (argc > 3) {
      for (int i=3; i < argc; i++) {
        if (strncmp(argv[i], "--out=", 6) == 0 && strlen(argv[i]) > 6) {
          dir = argv[i] + 6;
        } else if (strncmp(argv[i], "--strip=", 8) == 0 && strlen(argv[i]) > 8) {
          stripped = argv[i] + 8;
        } else {
          arg = argv[i];
          goto JMP_ERR;
        }
      }
    }

    return (ass::extract(argv[2], ass::M_EXTRACT, dir, stripped) == true) ? 0 : 1;
  }

  /* strip attachments from .ass file */
  if (strcmp(argv[1], "s") == 0 || strcmp(argv[1], "strip") == 0) {
    if (argc < 4) goto JMP_MISS;
    return (ass::extract(argv[2], ass::M_STRIP, {}, argv[3]) == true) ? 0 : 1;
  }

  /* attach files */
  if (strcmp(argv[1], "a") == 0 || strcmp(argv[1], "attach") == 0) {
    std::vector<std::string> vfonts, vgraphics;
    std::vector<std::string> *vec = &vfonts;

    if (argc < 6) goto JMP_MISS;

    if (strcmp(argv[4], "-f") != 0 && strcmp(argv[4], "-g") != 0) {
      arg = argv[4];
      goto JMP_ERR;
    }

    for (int i=4; i < argc; i++) {
      if (strcmp(argv[i], "-f") == 0) {
        vec = &vfonts;
      } else if (strcmp(argv[i], "-g") == 0) {
        vec = &vgraphics;
      } else {
        vec->push_back(argv[i]);
      }
    }

    return (ass::attach(argv[2], argv[3], vfonts, vgraphics) == true) ? 0 : 1;
  }

  arg = argv[1];

JMP_ERR:
  std::cerr << "error: incorrect argument given: " << arg << '\n' << std::endl;
  goto JMP_HELP;

JMP_MISS:
  std::cerr << "error: missing arguments\n" << std::endl;

JMP_HELP:
  std::cout << argv[0] << " -h|--help\n"
    << argv[0] << " a|attach input.ass output.ass -f fonts [...] -g graphics [...]\n"
    << argv[0] << " x|extract input.ass [--out=directory] [--strip=output.ass]\n"
    << argv[0] << " l|list input.ass\n"
    << argv[0] << " s|strip input.ass output.ass"
    << std::endl;

  return rv;
}

