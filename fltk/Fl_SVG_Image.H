#ifndef Fl_SVG_Image_H
#define Fl_SVG_Image_H

#ifndef Fl_H
# include <FL/Fl.H>
#endif
#ifndef Fl_RGB_Image_H
# include <FL/Fl_RGB_Image.H>
#endif

class Fl_SVG_Image : public Fl_RGB_Image
{
public:
  /**
    Set "rasterize" to 0 to disable rendering the image.
    This is useful if you only want to know width and height of the image.
  */
  Fl_SVG_Image(const char *filename, int rasterize=1);
  Fl_SVG_Image(const char *name_svg, char *svg_data, int rasterize=1);

  Fl_SVG_Image(float scale, const char *filename);
  Fl_SVG_Image(float scale, const char *name_svg, char *svg_data);
  Fl_SVG_Image(float scale_x, float scale_y, const char *filename);
  Fl_SVG_Image(float scale_x, float scale_y, const char *name_svg, char *svg_data);

private:
  float scale_x_, scale_y_;
  void load_svg_(const char *name_svg, char *svg_data, int rasterize);

public:
  float scale_x() {return scale_x_;}
  float scale_y() {return scale_y_;}
};

#endif // !Fl_SVG_Image_H

