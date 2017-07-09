#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <FL/filename.H>

#include <iostream>

static int event_x_pos, event_y_pos;

class move_box : public Fl_Box
{
  public:

    move_box(int X, int Y, int W, int H, const char *L=0)
      : Fl_Box(X, Y, W, H, L) { }

    virtual ~move_box() { }

    int handle(int event);
};

int move_box::handle(int event)
{
  int ret = Fl_Box::handle(event);
  switch (event)
  {
    case FL_PUSH:
      fl_cursor(FL_CURSOR_MOVE);
      event_x_pos = Fl::event_x();
      event_y_pos = Fl::event_y();
      ret = event;  /* must be non-zero */
      break;
    case FL_DRAG:
      do_callback();
      break;
    case FL_RELEASE:
      fl_cursor(FL_CURSOR_DEFAULT);
      break;
  }
  return ret;
}

static void move_box_cb(Fl_Widget *, void *v)
{
  Fl_Window *w = (Fl_Window *)v;
  int x = Fl::event_x_root() - event_x_pos;
  int y = Fl::event_y_root() - event_y_pos;
  w->position(x, y);
}


int main(int argc, char **argv)
{
  Fl_Window *win = new Fl_Window(200, 200, "move test");
  {
    move_box *b = new move_box(40, 40, 120, 120, "click here\nand move\nwindow around");
    b->box(FL_FLAT_BOX);
    b->color(FL_YELLOW);
    b->callback(move_box_cb, win);
  }
  win->end();
  win->show();

  return Fl::run();
}

