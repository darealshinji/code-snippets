#include "Fl_SVG_Image.H"

#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

#include <string.h>

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

void Fl_SVG_Image::load_svg_(const char *name_svg, char *svg_data, int rasterize)
{
  NSVGimage *image = NULL;
  NSVGrasterizer *r = NULL;
  const char *units = "px";
  float dpi, width, height;
  bool autow = false;
  bool autoh = false;

  dpi = 96.0f;

  if (svg_data) {
    size_t size = strlen(svg_data);
    char *tmp = new char[size];
    strncpy(tmp, svg_data, size-1);
    image = nsvgParse(tmp, units, dpi);
    delete[] tmp;
  } else if (name_svg) {
    image = nsvgParseFromFile(name_svg, units, dpi);
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
    autow = true;
  } else {
    width = (float)w_;
    scale_x_ = width / w_source_;
  }

  if (h_ == 0) {
    height = h_source_;
    scale_y_ = 1.0f;
  } else if (h_ <= -1) {
    autoh = true;
  } else {
    height = (float)h_;
    scale_y_ = height / h_source_;
  }

  if (autow && autoh) {
    width = w_source_;
    height = h_source_;
    scale_y_ = scale_x_ = 1.0f;
  } else if (autow) {
    scale_x_ = scale_y_;
    width = w_source_ * scale_x_;
  } else if (autoh) {
    scale_y_ = scale_x_;
    height = h_source_ * scale_y_;
  }

  w((int)width);
  h((int)height);
  d(4);

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

