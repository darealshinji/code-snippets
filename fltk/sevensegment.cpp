#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>

class SevSegDigit : public Fl_Widget
{
protected:
  void draw();

public:
  SevSegDigit(int X, int Y, int W, int H, const char *L = "8")
    : Fl_Widget(X,Y,W,H,L)
  {}
};

void SevSegDigit::draw()
{
  int a=1, b=1, c=1, d=1, e=0, f=1, g=1;
  const char *tmp = label();
  char n = tmp[0];

  if (n == '0') {
    e=1; g=0;
  } else if (n == '1') {
    a=0; d=0; f=0; g=0;
  } else if (n == '2') {
    c=0; e=1; f=0;
  } else if (n == '3') {
    f=0;
  } else if (n == '4') {
    a=0; d=0;
  } else if (n == '5') {
    b=0;
  } else if (n == '6') {
    b=0; e=1;
  } else if (n == '7') {
    d=0; f=0; g=0;
  } else if (n == '8') {
    e=1;
  } else if (n != '9') {
    a=0; b=0; c=0; d=0; f=0; g=0;
  }

  //draw_box(FL_FLAT_BOX, FL_BLACK);
  fl_color(FL_BLACK);

  int s1 = h()/10;
  int s2 = h()/2;

  if (a == 1) {
    fl_rectf(x(), y(), w(), s1);
  }
  if (b == 1) {
    fl_rectf(x()+w()-s1, y(), s1, s2);
  }
  if (c == 1) {
    fl_rectf(x()+w()-s1, y()+s2, s1, s2);
  }
  if (d == 1) {
    fl_rectf(x(), y()+h()-s1, w(), s1);
  }
  if (e == 1) {
    fl_rectf(x(), y()+s2, s1, s2);
  }
  if (f == 1) {
    fl_rectf(x(), y(), s1, s2);
  }
  if (g == 1) {
    fl_rectf(x(), y()+s2-s1/2, w(), s1);
  }
}

int main(int argc, char **argv)
{
  Fl_Window *win = new Fl_Window(300, 240);
  SevSegDigit *digit = new SevSegDigit(10, 10, 120, 200, "9");
  win->end();
  win->show(argc,argv);

  return Fl::run();
}
