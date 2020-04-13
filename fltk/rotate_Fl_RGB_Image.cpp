/* functions to rotate and mirror images for use with FLTK, including examples */

/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020, djcj <djcj@gmx.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_GIF_Image.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_XBM_Image.H>
#include <string.h>

/* fl_draw_pixmap.cxx */
//int fl_convert_pixmap(const char*const* cdata, uchar* out, Fl_Color bg);

enum {
  ROTATE_CLOCKWISE = 1,
  ROTATE_COUNTER_CLOCKWISE = 2,
  ROTATE_180 = 3,
  FLIP_VERTICAL = 4,
  FLIP_HORIZONTAL = 5
};

// w=10,h=8,d=1,ld=15
static const uchar rgb_array[] = {  // line data test
    0x00,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x01,0x02,0x03,0x04,0x05, // line data
    0xff,0xff,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0x06,0x07,0x08,0x09,0x0a, // line data
    0x00,0x00,0x00,0xff,0x00,0x00,0x00,0x00,0x00,0x00,
    0x01,0x02,0x03,0x04,0x05, // line data
    0xff,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0xff,0xff,
    0x06,0x07,0x08,0x09,0x0a, // line data
    0x00,0x00,0x00,0x00,0x00,0xff,0x00,0x00,0x00,0x00,
    0x01,0x02,0x03,0x04,0x05, // line data
    0xff,0xff,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0xff,
    0x06,0x07,0x08,0x09,0x0a, // line data
    0x00,0x00,0x00,0x00,0xff,0x00,0x00,0x00,0x00,0x00,
    0x01,0x02,0x03,0x04,0x05, // line data
    0xff,0xff,0xff,0x00,0xff,0xff,0xff,0xff,0xff,0xff,
    0x06,0x07,0x08,0x09,0x0a  // line data
};

static Fl_RGB_Image *modify_rgb_image(int method, Fl_RGB_Image *in)
{
  int i, j, x, y, w, h, d, ld, l;

  if (!in) {
    return NULL;
  }

  w = in->w();
  h = in->h();
  d = in->d();
  ld = in->ld();

  if (d < 1 || d > 4) {
    return NULL;
  }

  if ((ld > 0 && ld < w*d) || ld < 0) {
    return NULL;
  }

  uchar *out = new uchar[w*h*d];

  if (ld != 0) {
    ld -= w*d;
  }

  if (d == 1) {
    if (method == ROTATE_CLOCKWISE) {
      for (x=0, i=0; x < w; ++x) {
        for (y=h-1; y >= 0; --y, ++i) {
          out[i] = in->array[y*w + y*ld + x];
        }
      }

      if (w != h) {
        l = h; h = w; w = l;
      }
    }
    else if (method == ROTATE_COUNTER_CLOCKWISE) {
      for (x=w-1, i=0; x >= 0; --x) {
        for (y=0; y < h; ++y, ++i) {
          out[i] = in->array[y*w + y*ld + x];
        }
      }

      if (w != h) {
        l = h; h = w; w = l;
      }
    }
    else if (method == ROTATE_180) {
      if (ld == 0) {
        for (i=0, l=w*h; i < l; ++i) {
          out[i] = in->array[l - i];
        }
      } else {
        for (y=h-1, i=0; y >= 0; --y, ++i) {
          for (x=w-1, j=0; x >= 0; --x, ++j) {
            out[i*w + j] = in->array[y*w + y*ld + x];
          }
        }
      }
    }
    else if (method == FLIP_VERTICAL) {
      for (y=h-1, i=0; y >= 0; --y, ++i) {
        out[i*w] = in->array[y*ld + y*w];
      }
    }
    else if (method == FLIP_HORIZONTAL) {
      for (y=0; y < h; ++y) {
        for (x=w-1, i=0; x >= 0; --x, ++i) {
          out[i + y*w] = in->array[y*w + y*ld + x];
        }
      }
    } else {
      delete out;
      return NULL;
    }
  } else {
    if (method == ROTATE_CLOCKWISE) {
      for (x=0, i=0; x < w; ++x) {
        for (y=h-1; y >= 0; --y, i+=d) {
          memcpy(out + i, in->array + y*w*d + y*ld + x*d, d);
        }
      }

      if (w != h) {
        l = h; h = w; w = l;
      }
    }
    else if (method == ROTATE_COUNTER_CLOCKWISE) {
      for (x=w-1, i=0; x >= 0; --x) {
        for (y=0; y < h; ++y, i+=d) {
          memcpy(out + i, in->array + y*w*d + y*ld + x*d, d);
        }
      }

      if (w != h) {
        l = h; h = w; w = l;
      }
    }
    else if (method == ROTATE_180) {
      if (ld == 0) {
        for (i=0, l=w*h*d; i < l; i+=d) {
          memcpy(out + i, in->array + l - i - d, d);
        }
      } else {
        for (y=h-1, i=0; y >= 0; --y, ++i) {
          for (x=w-1, j=0; x >= 0; --x, j+=d) {
            memcpy(out + i*w*d + j, in->array + y*w*d + y*ld + x*d, d);
          }
        }
      }
    }
    else if (method == FLIP_VERTICAL) {
      for (y=h-1, i=0, l=w*d; y >= 0; --y, ++i) {
        memcpy(out + i*l, in->array + y*ld + y*l, l);
      }
    }
    else if (method == FLIP_HORIZONTAL) {
      for (y=0; y < h; ++y) {
        for (x=w-1, i=0; x >= 0; --x, i+=d) {
          memcpy(out + i + y*w*d, in->array + y*w*d + y*ld + x*d, d);
        }
      }
    } else {
      delete out;
      return NULL;
    }
  }

  Fl_RGB_Image *rgb = new Fl_RGB_Image(out, w, h, d);
  rgb->alloc_array = 1;

  return rgb;
}

static Fl_RGB_Image *convert_bitmap_to_rgb(int method, Fl_Bitmap *in)
{
  const uchar col0 = 0x00, col1 = 0xff;
  int w, h, x, y, i, b, l;

  if (!in) {
    return NULL;
  }

  w = in->w();
  h = in->h();

  uchar *out = new uchar[w*h];
  uchar *out2 = NULL;
  uchar line[w];

  if (method == ROTATE_180) {
    for (y=0, i=0, l=w*h-1; y < h; ++y) {
      for (x=0; x < w; x+=8, ++i) {
        for (b=0; b < 8; ++b) {
          out[l - (y*w + x + b)] = (0 != (in->array[i] & (1 << b))) ? col0 : col1;
        }
      }
    }
  }
  else if (method == FLIP_VERTICAL) {
    for (y=0, i=0; y < h; ++y) {
      for (x=0; x < w; x+=8, ++i) {
        for (b=0; b < 8; ++b) {
          line[x + b] = (0 != (in->array[i] & (1 << b))) ? col0 : col1;
        }
      }
      memcpy(out + (h-1-y)*w, line, w);
    }
  }
  else if (method == FLIP_HORIZONTAL) {
    for (y=0, i=0; y < h; ++y) {
      for (x=0; x < w; x+=8, ++i) {
        for (b=0; b < 8; ++b) {
          line[w - 1 - x - b] = (0 != (in->array[i] & (1 << b))) ? col0 : col1;
        }
      }
      memcpy(out + y*w, line, w);
    }
  }
  else {
    /* regular convertion to RGB (d() == 1) */
    for (y=0, i=0; y < h; ++y) {
      for (x=0; x < w; x+=8, ++i) {
        for (b=0; b < 8; ++b) {
          out[y*w + x + b] = (0 != (in->array[i] & (1 << b))) ? col0 : col1;
        }
      }
    }
  }

  if (method == ROTATE_CLOCKWISE) {
    out2 = new uchar[w*h];

    for (x=0, i=0; x < w; ++x) {
      for (y=h-1; y >= 0; --y, ++i) {
        out2[i] = out[y*w + x];
      }
    }

    if (w != h) {
      l = h; h = w; w = l;
    }

    delete out;
    out = out2;
  }
  else if (method == ROTATE_COUNTER_CLOCKWISE) {
    out2 = new uchar[w*h];

    for (x=w-1, i=0; x >= 0; --x) {
      for (y=0; y < h; ++y, ++i) {
        out2[i] = out[y*w + x];
      }
    }

    if (w != h) {
      l = h; h = w; w = l;
    }

    delete out;
    out = out2;
  }

  Fl_RGB_Image *rgb = new Fl_RGB_Image(out, w, h, 1);
  rgb->alloc_array = 1;

  return rgb;
}

int main(void)
{
  Fl_Double_Window *win = new Fl_Double_Window(400, 400, "Rotate Test");
  Fl_Box *b1 = new Fl_Box(0, 0, win->w(), win->h()/2);
  Fl_Box *b2 = new Fl_Box(0, win->h()/2 - 20, win->w(), win->h()/2);

  //Fl_PNG_Image *rgb = new Fl_PNG_Image("test.png");  // d()==4
  //Fl_JPEG_Image *rgb = new Fl_JPEG_Image("test.jpg");  // d()==3
  //Fl_RGB_Image *rgb = new Fl_RGB_Image(rgb_array, 10, 8, 1, 15);

  Fl_XBM_Image *xbm = new Fl_XBM_Image("srs.xbm");
  Fl_RGB_Image *rgb = convert_bitmap_to_rgb(ROTATE_CLOCKWISE, xbm);
  delete xbm;

  //Fl_GIF_Image *gif = new Fl_GIF_Image("test.gif");
  //Fl_RGB_Image *rgb = new Fl_RGB_Image(gif, Fl_Color(0));
  //delete gif;

  Fl_RGB_Image *rot = modify_rgb_image(ROTATE_COUNTER_CLOCKWISE, rgb);

  int sc = 2;

  if (rgb) {
    b1->image(rgb->copy(rgb->w()*sc, rgb->h()*sc));
    delete rgb;
  }

  if (rot) {
    b2->image(rot->copy(rot->w()*sc, rot->h()*sc));
    delete rot;
  }

  win->end();
  win->show();
  Fl::run();

  return 0;
}
