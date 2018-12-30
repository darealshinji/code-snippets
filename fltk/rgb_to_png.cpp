/* save Fl_RGB_Image data to PNG file */

#include <FL/Fl.H>
#include <FL/Fl_RGB_Image.H>
#include <setjmp.h>
#include <stdio.h>
#include <png.h>

// only needed for main()
#include <FL/Fl_XPM_Image.H>
#include <iostream>


static int rgb_to_png(const char *file, Fl_RGB_Image *rgb)
{
  FILE *fp = NULL;
  png_struct *png = NULL;
  png_info *info = NULL;
  png_byte *row;
  const unsigned char *rgba;
  const int compression_level = 0;  /* 0-9 */
  int w, h;

  if ((fp = fopen(file, "wb")) == NULL) {
    return 1;
  }

  if ((png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL) {
    fclose(fp);
    return 1;
  }

  if ((info = png_create_info_struct(png)) == NULL) {
    fclose(fp);
    png_destroy_write_struct(&png, NULL);
    return 1;
  }

  if (setjmp(png_jmpbuf(png)) != 0) {
    fclose(fp);
    png_free_data(png, info, PNG_FREE_ALL, -1);
    png_destroy_write_struct(&png, NULL);
    return 1;
  }

  png_init_io(png, fp);
  png_set_compression_level(png, compression_level);

  w = rgb->w();
  h = rgb->h();
  png_set_IHDR(png, info, w, h, 8,
               PNG_COLOR_TYPE_RGB_ALPHA,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  png_write_info(png, info);

  row = new png_byte[w * 4 * sizeof(png_byte)];
  rgba = reinterpret_cast<const unsigned char *>(*rgb->data());

  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      png_byte *px = &row[x * 4];
      int i = (y*w + x) * 4;
      px[0] = rgba[i];
      px[1] = rgba[i + 1];
      px[2] = rgba[i + 2];
      px[3] = rgba[i + 3];
    }
    png_write_row(png, row);
  }
  png_write_end(png, NULL);
  fclose(fp);

  png_free_data(png, info, PNG_FREE_ALL, -1);
  png_destroy_write_struct(&png, NULL);
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

  int rv = rgb_to_png("out.png", rgb);

  Fl::run();

  delete xpm;
  delete rgb;

  return rv;
}

