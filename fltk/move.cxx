#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>


class movebox : public Fl_Box
{
private:

    int m_event_x = 0;
    int m_event_y = 0;

public:

    movebox(int X, int Y, int W, int H, const char *L=0)
    : Fl_Box(X, Y, W, H, L) {}

    virtual ~movebox() {}

protected:

    int handle(int e)
    {
        int rv = Fl_Box::handle(e);

        switch (e) {
            case FL_PUSH:
                fl_cursor(FL_CURSOR_MOVE);
                m_event_x = Fl::event_x();
                m_event_y = Fl::event_y();
                rv = e;  /* must be non-zero */
                break;
            case FL_DRAG:
                window()->position(
                    Fl::event_x_root() - m_event_x,
                    Fl::event_y_root() - m_event_y);
                break;
            case FL_RELEASE:
                fl_cursor(FL_CURSOR_DEFAULT);
                break;
            default:
                break;
        }

        return rv;
    }
};


int main()
{
    Fl_Window win(200, 200, "move test");
    movebox box(40, 40, 120, 120, "click here\nand move\nwindow around");
    box.box(FL_FLAT_BOX);
    box.color(FL_YELLOW);
    win.end();
    win.show();
    return Fl::run();
}
