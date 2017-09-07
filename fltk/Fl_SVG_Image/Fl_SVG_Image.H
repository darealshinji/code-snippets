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
  /**
    To keep the original width or height, set W or H to 0.
    A negative value will stretch the width or height automatically to keep
    the original aspect ratio.
    Fl_SVG_Image(300,0,"test.svg) for example will stretch a 48x48 SVG to
    300x48, Fl_SVG_Image(300,-1,"test.svg) will stretch it to 300x300.
  */
  Fl_SVG_Image(int W, int H, const char *filename);
  Fl_SVG_Image(int W, int H, const char *name_svg, char *svg_data);

private:
  int w_, h_;
  float scale_x_, scale_y_, w_source_, h_source_;
  void load_svg_(const char *name_svg, char *svg_data, int rasterize);

public:
  float scale()   { return (scale_x_ == scale_y_) ? scale_x_ : (scale_x_ + scale_y_) * 0.5f; }
  float scale_x() { return scale_x_; }
  float scale_y() { return scale_y_; }
  /**
    Return width and height set in the original SVG.
  */
  int w_source() { return (int)w_source_; }
  int h_source() { return (int)h_source_; }
};

#endif // !Fl_SVG_Image_H

