/* save Fl_RGB_Image data to ARGB Windows bitmap file */

#include <FL/Fl.H>
#include <FL/Fl_RGB_Image.H>
#include <fstream>
#include <string>

// only needed for main()
#include <FL/Fl_XPM_Image.H>
#include <iostream>


static int rgb_to_bitmap(const char *file, Fl_RGB_Image *rgb)
{
  std::ofstream out;
  std::string header;
  const unsigned char *rgba;
  unsigned char *row;
  int w, h, zero, offset, one, bpp, infohdr_size, row_size;
  size_t data_len, fsize;

  out.open(file, std::ios::out|std::ios::binary);
  if (!out.is_open()) {
    return 1;
  }

  w = rgb->w();
  h = rgb->h();
  data_len = w * h * rgb->d();

  if (data_len % 4 != 0) {
    return 1;
  }

#define ADD(a,b)  header.append(reinterpret_cast<const char *>(a), b)

  /* https://en.wikipedia.org/wiki/BMP_file_format#Bitmap_file_header */

  /* file header */
  offset = 54;        /* file header (14 bytes) + info header (40 bytes) */
  fsize = offset + data_len;
  zero = 0;
  ADD(&"BM", 2);      /* magic bytes */
  ADD(&fsize, 4);     /* file size */
  ADD(&zero, 4);      /* reserved */
  ADD(&offset, 4);    /* pixel data offset */

  /* BITMAPINFOHEADER */
  infohdr_size = 40;
  one = 1;
  bpp = 32;
  ADD(&infohdr_size, 4);    /* size of this header (40 bytes) */
  ADD(&w, 4);               /* width */
  ADD(&h, 4);               /* height */
  ADD(&one, 2);             /* number of color planes, must be 1 */
  ADD(&bpp, 2);             /* bits per pixel */
  /* write 6*4 zero bytes:
   * - compression method (BI_RGB = 0)
   * - raw bitmap data size; can be 0 for BI_RGB
   * - horizontal resolution (pixel per metre) (???)
   * - vertical resolution (pixel per metre) (???)
   * - number of colors in color palette (0 to default to 2^n)
   * - number of important colors used, or 0 when every color is important; generally ignored
   */
  ADD(&zero, 24);

#undef ADD

  out.write(header.c_str(), offset);

  /* write pixel data (must be rearranged a bit) */
  row_size = w * 4;
  row = new unsigned char[row_size];
  rgba = reinterpret_cast<const unsigned char *>(*rgb->data());

  for (int y = h - 1; y >= 0; --y) {
    for (int x = 0; x < w; ++x) {
      unsigned char *px = &row[x * 4];
      int i = (y*w + x) * 4;
      px[0] = rgba[i + 2];
      px[1] = rgba[i + 1];
      px[2] = rgba[i];
      px[3] = rgba[i + 3];
    }
    out.write(reinterpret_cast<const char *>(row), row_size);
  }

  out.close();
  delete row;

  return 0;
}

int main(void)
{
  Fl_XPM_Image *xpm = new Fl_XPM_Image("crc32_check.xpm");
  if (!xpm) {
    std::cerr << "error: Fl_XPM_Image()" << std::endl;
    return 1;
  }
  Fl_RGB_Image *rgb = new Fl_RGB_Image(xpm, Fl_Color(0));

  int rv = rgb_to_bitmap("out.bmp", rgb);

  Fl::run();

  delete xpm;
  delete rgb;

  return rv;
}

