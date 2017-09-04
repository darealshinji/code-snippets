#include "Fl_SVG_Image.H"

#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

#include <string.h>

Fl_SVG_Image::Fl_SVG_Image(const char *filename, int rasterize)
 : Fl_RGB_Image(0,0,0)
{
  scale_x_ = scale_y_ = 1.0f;
  load_svg_(filename, NULL, rasterize);
}

Fl_SVG_Image::Fl_SVG_Image(const char *name_svg, char *svg_data, int rasterize)
 : Fl_RGB_Image(0,0,0)
{
  scale_x_ = scale_y_ = 1.0f;
  load_svg_(name_svg, svg_data, rasterize);
}

Fl_SVG_Image::Fl_SVG_Image(float scale, const char *filename)
 : Fl_RGB_Image(0,0,0)
{
  scale_x_ = scale_y_ = scale;
  load_svg_(filename, NULL, 1);
}

Fl_SVG_Image::Fl_SVG_Image(float scale, const char *name_svg, char *svg_data)
 : Fl_RGB_Image(0,0,0)
{
  scale_x_ = scale_y_ = scale;
  load_svg_(name_svg, svg_data, 1);
}

Fl_SVG_Image::Fl_SVG_Image(float scale_x, float scale_y, const char *filename)
 : Fl_RGB_Image(0,0,0)
{
  scale_x_ = scale_x;
  scale_y_ = scale_y;
  load_svg_(filename, NULL, 1);
}

Fl_SVG_Image::Fl_SVG_Image(float scale_x, float scale_y, const char *name_svg, char *svg_data)
 : Fl_RGB_Image(0,0,0)
{
  scale_x_ = scale_x;
  scale_y_ = scale_y;
  load_svg_(name_svg, svg_data, 1);
}

void Fl_SVG_Image::load_svg_(const char *name_svg, char *svg_data, int rasterize)
{
  NSVGimage *image = NULL;
  NSVGrasterizer *r;
  const char *units = "px";
  float dpi, scale_x, scale_y;

  dpi = 96.0f;
  scale_x = (scale_x_ <= 0.0f) ? 1.0f : scale_x_;
  scale_y = (scale_y_ <= 0.0f) ? 1.0f : scale_y_;

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

  w((int)(image->width * scale_x));
  h((int)(image->height * scale_y));

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
  nsvgRasterizeFull(r, image, 0, 0, scale_x, scale_y, (uchar *)array, w(), h(), w() * d());

  nsvgDeleteRasterizer(r);
  nsvgDelete(image);
}

