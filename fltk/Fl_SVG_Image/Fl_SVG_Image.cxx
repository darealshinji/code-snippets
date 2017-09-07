#include "Fl_SVG_Image.H"

#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

#include <string.h>

//#include "config.h"
#define HAVE_LIBZ 1

#ifdef HAVE_LIBZ
#include <zlib.h>
#endif


Fl_SVG_Image::Fl_SVG_Image(const char *filename, int rasterize)
 : Fl_RGB_Image(0,0,0)
{
  w_ = h_ = -1;
  scale_x_ = scale_y_ = 1.0f;
  load_svg_(filename, NULL, rasterize);
}

Fl_SVG_Image::Fl_SVG_Image(const char *name_svg, char *svg_data, int rasterize)
 : Fl_RGB_Image(0,0,0)
{
  w_ = h_ = -1;
  scale_x_ = scale_y_ = 1.0f;
  load_svg_(name_svg, svg_data, rasterize);
}

Fl_SVG_Image::Fl_SVG_Image(float scale, const char *filename)
 : Fl_RGB_Image(0,0,0)
{
  w_ = h_ = -1;
  scale_x_ = scale_y_ = scale;
  load_svg_(filename, NULL, 1);
}

Fl_SVG_Image::Fl_SVG_Image(float scale, const char *name_svg, char *svg_data)
 : Fl_RGB_Image(0,0,0)
{
  w_ = h_ = -1;
  scale_x_ = scale_y_ = scale;
  load_svg_(name_svg, svg_data, 1);
}

Fl_SVG_Image::Fl_SVG_Image(int W, int H, const char *filename)
 : Fl_RGB_Image(0,0,0)
{
  w_ = W; h_ = H;
  scale_x_ = scale_y_ = 1.0f;
  load_svg_(filename, NULL, 1);
}

Fl_SVG_Image::Fl_SVG_Image(int W, int H, const char *name_svg, char *svg_data)
 : Fl_RGB_Image(0,0,0)
{
  w_ = W; h_ = H;
  scale_x_ = scale_y_ = 1.0f;
  load_svg_(name_svg, svg_data, 1);
}

#define CHUNK_SIZE 0x4000  /* memory chunk size used by zlib */
#define SVG_UNITS  "px"    /* units passed to NanoSVG */
#define SVG_DPI    96.0f   /* DPI (dots-per-inch) used for unit conversion */
#define SVG_DEPTH  4       /* image depth */

void Fl_SVG_Image::load_svg_(const char *name_svg, char *svg_data, int rasterize)
{
  NSVGimage *image = NULL;
  NSVGrasterizer *r = NULL;
  float width, height;
  bool auto_w = false;
  bool auto_h = false;

  if (svg_data) {
    size_t size = strlen(svg_data);
    char *tmp = new char[size];
    strncpy(tmp, svg_data, size-1);
    image = nsvgParse(tmp, SVG_UNITS, SVG_DPI);
    delete[] tmp;
  } else if (name_svg) {
#ifdef HAVE_LIBZ
    size_t length = strlen(name_svg);
    if ((length > 7 && strcmp(".svg.gz", name_svg+length-7) == 0) ||
        (length > 5 && strcmp(".svgz", name_svg+length-5) == 0))
    {
      FILE *file;
      char *string = NULL;
      Bytef in[CHUNK_SIZE];
      Bytef out[CHUNK_SIZE];
      z_stream strm;

      strm.next_in = in;
      strm.zalloc = Z_NULL;
      strm.zfree = Z_NULL;
      strm.opaque = Z_NULL;

      if (inflateInit2(&strm, 15|32) != Z_OK) {
        goto stop;
      }

      if ((file = fopen(name_svg, "rb")) == NULL) {
        goto stop;
      }

      while (1) {
        strm.avail_in = (uInt)fread(in, sizeof(Bytef), sizeof(in), file);
        if (ferror(file)) {
          fclose(file);
          inflateEnd(&strm);
          goto stop;
        }

        do {
          strm.avail_out = CHUNK_SIZE;
          strm.next_out = out;
          int status = inflate(&strm, Z_NO_FLUSH);
          if (status != Z_OK && status != Z_STREAM_END) {
            fclose(file);
            inflateEnd(&strm);
            goto stop;
          }
        }
        while (strm.avail_out == 0);

        if (feof(file)) {
          string = new char[strm.total_out+1];
          strncpy(string, (const char *)out, (size_t)strm.total_out);
          break;
        }
      }
      inflateEnd(&strm);

      if (fclose(file)) {
        if (string) { delete[] string; }
        goto stop;
      }

      if (string) {
        string[strm.total_out+1] = '\0';
        image = nsvgParse(string, SVG_UNITS, SVG_DPI);
        delete[] string;
      }
    }
    else
#endif //HAVE_LIBZ
    {
      /* uncompressed SVG */
      image = nsvgParseFromFile(name_svg, SVG_UNITS, SVG_DPI);
    }
  }

  if (!image) {
    goto stop;
  }

  w_source_ = image->width;
  h_source_ = image->height;

  if (w_ == 0) {
    width = w_source_;
    scale_x_ = 1.0f;
  } else if (w_ <= -1) {
    auto_w = true;
  } else {
    width = (float)w_;
    scale_x_ = width / w_source_;
  }

  if (h_ == 0) {
    height = h_source_;
    scale_y_ = 1.0f;
  } else if (h_ <= -1) {
    auto_h = true;
  } else {
    height = (float)h_;
    scale_y_ = height / h_source_;
  }

  if (auto_w && auto_h) {
    width = w_source_;
    height = h_source_;
    scale_y_ = scale_x_ = 1.0f;
  } else if (auto_w) {
    scale_x_ = scale_y_;
    width = w_source_ * scale_x_;
  } else if (auto_h) {
    scale_y_ = scale_x_;
    height = h_source_ * scale_y_;
  }

  w((int)width);
  h((int)height);
  d(SVG_DEPTH);

  if (rasterize == 0) {
    goto stop;
  }

  if ((size_t)(w() * h() * d()) > max_size()) {
    goto stop;
  }

  array = new uchar[w() * h() * d()];
  alloc_array = 1;
  r = nsvgCreateRasterizer();
  nsvgRasterizeFull(r, image, 0, 0, scale_x_, scale_y_, (uchar *)array, w(), h(), w() * d());

stop:
  if (image) { nsvgDelete(image); }
  if (r) { nsvgDeleteRasterizer(r); }
  return;
}

