#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_SVG_Image.H>
#include <FL/fl_draw.H>

static inline bool same_image_format(Fl_RGB_Image *a, Fl_RGB_Image *b)
{
  return (a && b && !a->fail() && !b->fail() &&
          a->w() == b->w() && a->h() == b->h() &&
          a->d() == b->d() && a->ld() == b->ld());
}

// blend/overlay up to 3 Fl_RGB_Image* images in RGBA format
static Fl_RGB_Image *blend_rgba(Fl_RGB_Image *inBg, Fl_RGB_Image *inFg, Fl_RGB_Image *inFg2=NULL)
{
  // background image is needed
  if (!inBg || inBg->fail()) {
    return NULL;
  }

  if (!inFg || inFg->fail()) {
    if (inFg2 && !inFg2->fail()) {
      // take over pointer
      inFg = inFg2;
      inFg2 = NULL;
    } else {
      // no foreground images provided
      return NULL;
    }
  }

  const int w = inBg->w();
  int ld = inBg->ld();

  // d() must be 4, ld() must be formatted correctly
  if (inBg->d() != 4 || (ld > 0 && ld < w*4) || ld < 0) {
    return NULL;
  }

  // compare image specs

  if (!same_image_format(inBg, inFg)) {
    return NULL;
  }

  if (!same_image_format(inFg, inFg2)) {
    inFg2 = NULL;
  }

  // don't calculate with the full line data
  if (ld != 0) {
    ld -= w*4;
  }

  const int h = inBg->h();
  uchar *out = new uchar[w*h*4];

  // http://de.voidcc.com/question/p-shsavrtq-bz.html
  if (inFg2) {
    for (int x=0, i=0; x < w; ++x) {
      for (int y=0; y < h; ++y, i+=4) {
        const int off = i + y*ld;
        const uchar *bg = inBg->array + off;
        const uchar *fg = inFg->array + off;
        const uchar *fg2 = inFg2->array + off;
        uint alpha = fg[3] + 1;
        uchar *px = out + i;

        if (alpha == 1) {
          px[0] = bg[0];
          px[1] = bg[1];
          px[2] = bg[2];
          px[3] = bg[3];
        } else {
          const uint inv_alpha = 256 - fg[3];
          px[0] = (alpha * fg[0] + inv_alpha * bg[0]) >> 8;
          px[1] = (alpha * fg[1] + inv_alpha * bg[1]) >> 8;
          px[2] = (alpha * fg[2] + inv_alpha * bg[2]) >> 8;
          px[3] = (alpha * fg[3] + inv_alpha * bg[3]) >> 8;
        }

        if ((alpha = fg2[3] + 1) != 1) {
          const uint inv_alpha = 256 - fg2[3];
          px[0] = (alpha * fg2[0] + inv_alpha * px[0]) >> 8;
          px[1] = (alpha * fg2[1] + inv_alpha * px[1]) >> 8;
          px[2] = (alpha * fg2[2] + inv_alpha * px[2]) >> 8;
          px[3] = (alpha * fg2[3] + inv_alpha * px[3]) >> 8;
        }
      }
    }
  } else {
    for (int x=0, i=0; x < w; ++x) {
      for (int y=0; y < h; ++y, i+=4) {
        const int off = i + y*ld;
        const uchar *bg = inBg->array + off;
        const uchar *fg = inFg->array + off;
        const uint alpha = fg[3] + 1;
        uchar *px = out + i;

        if (alpha == 1) {
          px[0] = bg[0];
          px[1] = bg[1];
          px[2] = bg[2];
          px[3] = bg[3];
        } else {
          const uint inv_alpha = 256 - fg[3];
          px[0] = (alpha * fg[0] + inv_alpha * bg[0]) >> 8;
          px[1] = (alpha * fg[1] + inv_alpha * bg[1]) >> 8;
          px[2] = (alpha * fg[2] + inv_alpha * bg[2]) >> 8;
          px[3] = (alpha * fg[3] + inv_alpha * bg[3]) >> 8;
        }
      }
    }
  }

  Fl_RGB_Image *rgba = new Fl_RGB_Image(out, w, h, 4);
  rgba->alloc_array = 1;

  return rgba;
}

int main()
{
  const int w = 64;
  const int h = 64;

  Fl_Double_Window win(500, 300, 200, 200, "Test");
  Fl_Box box(0, 0, win.w(), win.h());

  Fl_SVG_Image *icn = new Fl_SVG_Image("icons/Folder_generic.svg", NULL);
  Fl_SVG_Image *lnk = new Fl_SVG_Image("icons/Overlay_link.svg", NULL);
  Fl_SVG_Image *lck = new Fl_SVG_Image("icons/Overlay_padlock.svg", NULL);

  icn->proportional = lnk->proportional = lck->proportional = false;
  icn->resize(w, h);
  lnk->resize(w, h);
  lck->resize(w, h);

  Fl_RGB_Image *img = blend_rgba(icn, lnk, lck);
  delete icn;
  delete lnk;
  delete lck;

  if (img) box.image(img);

  win.end();
  win.show();
  Fl::run();

  if (img) delete img;

  return 0;
}
