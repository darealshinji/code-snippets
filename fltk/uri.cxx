#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <FL/filename.H>

#include <iostream>


class uri_box : public Fl_Box
{
  public:

    uri_box(int X, int Y, int W, int H, const char *L=0)
      : Fl_Box(X, Y, W, H, L) { }

    virtual ~uri_box() { }

    int handle(int event);
}; 

int uri_box::handle(int event)
{
  int ret = Fl_Box::handle(event);
  switch (event)
  {
    case FL_PUSH:
      do_callback();
      break;
    case FL_MOVE:
      fl_cursor(FL_CURSOR_HAND);
      break;
    case FL_LEAVE:
      fl_cursor(FL_CURSOR_DEFAULT);
      break;
  }
  return ret;
}

static void open_uri_cb(Fl_Widget *, void *v)
{
  const char *uri = (char *)v;
  char errmsg[512];

  if (!fl_open_uri(uri, errmsg, sizeof(errmsg)))
  {
    std::cerr << "Error: " << errmsg << std::endl;
  }
}

//#define DEBUG 1  /* makes boxes visible */
#if (DEBUG == 1)
#  define SETBOXTYPE(o)  o->box(FL_FLAT_BOX); o->color(FL_GREEN)
#else
#  define SETBOXTYPE(o)  o->box(FL_NO_BOX)
#endif

int main(int argc, char **argv)
{
  Fl_Window *win = new Fl_Window(200, 150, "uri test");
  {
    { /* website */
      uri_box *o = new uri_box(10, 10, 78, 18, "www.fltk.org");
      SETBOXTYPE(o);
      o->labelcolor(FL_BLUE);
      o->callback(open_uri_cb, (void *)"http://www.fltk.org"); }

    { /* email address */
      uri_box *o = new uri_box(10, 40, 78, 18, "info@@fsf.org");
      SETBOXTYPE(o);
      o->labelcolor(FL_BLUE);
      o->callback(open_uri_cb, (void *)"mailto:info@fsf.org"); }

    { /* file */
      uri_box *o = new uri_box(10, 70, 66, 18, "/etc/fstab");
      SETBOXTYPE(o);
      o->callback(open_uri_cb, (void *)"file:///etc/fstab"); }

    { /* directory */
      uri_box *o = new uri_box(10, 100, 90, 18, "/usr/local/bin");
      SETBOXTYPE(o);
      o->callback(open_uri_cb, (void *)"file:///usr/local/bin"); }
  }
  win->end();
  win->show();

  return Fl::run();
}

