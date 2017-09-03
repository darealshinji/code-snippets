#include "Fl_SVG_Image.H"

#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

#include <string.h>

Fl_SVG_Image::Fl_SVG_Image(const char *filename, int rasterize)
 : Fl_RGB_Image(0,0,0)
{
  scale_ = 1.0f;
  load_svg_(filename, NULL, rasterize);
}

Fl_SVG_Image::Fl_SVG_Image(const char *name_svg, char *svg_data, int rasterize)
 : Fl_RGB_Image(0,0,0)
{
  scale_ = 1.0f;
  load_svg_(name_svg, svg_data, rasterize);
}

Fl_SVG_Image::Fl_SVG_Image(float scale, const char *filename)
 : Fl_RGB_Image(0,0,0)
{
  scale_ = scale;
  load_svg_(filename, NULL, 1);
}

Fl_SVG_Image::Fl_SVG_Image(float scale, const char *name_svg, char *svg_data)
 : Fl_RGB_Image(0,0,0)
{
  scale_ = scale;
  load_svg_(name_svg, svg_data, 1);
}

void Fl_SVG_Image::load_svg_(const char *name_svg, char *svg_data, int rasterize)
{
  NSVGimage *image = NULL;
  NSVGrasterizer *r;
  const char *units = "px";
  float dpi = 96.0f;

  w(0);
  h(0);
  d(4);

  r = nsvgCreateRasterizer();

  if (svg_data) {
    size_t size = strlen(svg_data);
    char *tmp = new char[size];
    memcpy(tmp, svg_data, size);
    image = nsvgParse(tmp, units, dpi);
    delete[] tmp;
  } else if (name_svg) {
    image = nsvgParseFromFile(name_svg, units, dpi);
  }

  if (!image) {
    nsvgDeleteRasterizer(r);
    return;
  }

  w((int)(image->width * scale_));
  h((int)(image->height * scale_));

  if (rasterize == 0) {
    nsvgDelete(image);
    nsvgDeleteRasterizer(r);
    return;
  }

  if ((size_t)(w() * h() * d()) > max_size()) {
    nsvgDeleteRasterizer(r);
    nsvgDelete(image);
    return;
  }

  array = new uchar[w() * h() * d()];
  alloc_array = 1;
  nsvgRasterize(r, image, 0, 0, scale_, (uchar *)array, w(), h(), w() * d());

  nsvgDeleteRasterizer(r);
  nsvgDelete(image);
}

