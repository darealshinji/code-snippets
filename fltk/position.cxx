#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>

class my_window : public Fl_Double_Window
{
public:

  my_window(int X, int Y, int W, int H, const char *L=0)
    : Fl_Double_Window(X, Y, W, H, L) { }

  my_window(int W, int H, const char *L=0)
    : Fl_Double_Window(W, H, L) { }

  virtual ~my_window() { }

  void position(int X, int Y) { resize(X, Y, w(), h()); }
  void position(Fl_Align pos);
};

void my_window::position(Fl_Align pos)
{
/*
               TOP_LEFT        TOP       TOP_RIGHT
               +---------------------------------+
       LEFT_TOP|                                 |RIGHT_TOP
               |                                 |
           LEFT|             CENTER              |RIGHT
               |                                 |
    LEFT_BOTTOM|                                 |RIGHT_BOTTOM
               +---------------------------------+
               BOTTOM_LEFT   BOTTOM   BOTTOM_RIGHT
*/

  if (pos & FL_ALIGN_INSIDE) {
    pos &= ~FL_ALIGN_INSIDE;
  }

  switch (pos) {
    case FL_ALIGN_CENTER:
      position((Fl::w() - w()) / 2, (Fl::h() - h()) / 2);
      break;

    case FL_ALIGN_TOP:
      position((Fl::w() - w()) / 2, 1);
      break;

    case FL_ALIGN_LEFT_TOP:
    case FL_ALIGN_TOP_LEFT:
      position(1, 1);
      break;

    case FL_ALIGN_RIGHT_TOP:
    case FL_ALIGN_TOP_RIGHT:
      position(Fl::w() - w(), 1);
      break;

    case FL_ALIGN_LEFT:
      position(1, (Fl::h() - h()) / 2);
      break;

    case FL_ALIGN_RIGHT:
      position(Fl::w() - w(), (Fl::h() - h()) / 2);
      break;

    case FL_ALIGN_BOTTOM:
      position((Fl::w() - w()) / 2,  Fl::h() - h());
      break;

    case FL_ALIGN_LEFT_BOTTOM:
    case FL_ALIGN_BOTTOM_LEFT:
      position(1,  Fl::h() - h());
      break;

    case FL_ALIGN_RIGHT_BOTTOM:
    case FL_ALIGN_BOTTOM_RIGHT:
      position(Fl::w() - w(),  Fl::h() - h());
      break;

    default:
      break;
  }
}


int main(void)
{
  my_window *win = new my_window(200, 200, "test");
  win->end();
  win->position(FL_ALIGN_CENTER);
  win->show();

  return Fl::run();
}

