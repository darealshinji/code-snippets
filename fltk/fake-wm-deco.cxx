#include <FL/Fl.H>
#include <FL/Fl_Bitmap.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Tooltip.H>
#include <FL/fl_draw.H>
#include <FL/filename.H>

#define USE_SIMPLE_MENU_BUTTON
#ifdef USE_SIMPLE_MENU_BUTTON
# include "simple_menu_button.cxx"
#else
# include <FL/Fl_Menu_Button.H>
#endif

/* icon64_png, icon64_png_len, icon24_png, icon24_png_len */
#include "icons.h"

class move_box : public Fl_Box {
  public:
    move_box(int X, int Y, int W, int H, const char *L=0) : Fl_Box(X, Y, W, H, L) { }
    virtual ~move_box() { }
    int handle(int event);
};

class resize_box : public Fl_Box {
  public:
    resize_box(int X, int Y, int W, int H, const char *L=0) : Fl_Box(X, Y, W, H, L) { }
    virtual ~resize_box() { }
    int handle(int event);
};

static Fl_Double_Window *mainw;
static Fl_Box *maximize_icon, *restore_icon_a, *restore_icon_b;
static Fl_Box *maximize_tooltip_overlay, *b_resize_icon;
static resize_box *b_resize;

static const int minw = 400;
static const int minh = 200;
static const int winw_default = 640;
static const int winh_default = 480;

static int event_x_pos, event_y_pos, winx_restore, winy_restore, x_diff, y_diff;
static int winw_restore, winh_restore;
static int winw = winw_restore = winw_default;
static int winh = winh_restore = winh_default;
static bool window_movable = true;
static bool window_maximized = false;

/* set true to enable a "Restore default size" button */
static bool default_size_button = true;

static const char *win_label = "Window title";

/* tooltips */
static bool tooltips = true;
static const char *iconize_tt = "Minimize";
static const char *default_size_tt = "Restore default size";
static const char *maximize_tt = "Full size";
static const char *restore_tt = "Restore size";
static const char *close_tt = "Close";
static const char *resize_tt = "Resize";

static Fl_Color main_color = fl_rgb_color(224, 223, 222); /* light gray */
static Fl_Color frame_color = fl_rgb_color(0, 120, 215);  /* light blue */
static Fl_Color frame_color2 = FL_WHITE;
static Fl_Color bt_color = fl_lighter(frame_color);  /* lighter blue */
static Fl_Color bt_close_color = fl_lighter(FL_RED);

static void window_restored_settings();
static void window_maximized_settings();
static void move_box_cb(Fl_Widget *);
static void resize_cb(Fl_Widget *);
static void iconize_cb(Fl_Widget *, void*);
static void default_size_cb(Fl_Widget *, void*);
static void maximize_restore_cb(Fl_Widget *, void*);
static void close_cb(Fl_Widget *, void*);

static Fl_Menu_Item titlebar_menu_items[] =
{
  { iconize_tt,      0, iconize_cb,          0, 0, FL_NORMAL_LABEL, 0, 14, frame_color2 },
  { default_size_tt, 0, default_size_cb,     0, 0, FL_NORMAL_LABEL, 0, 14, frame_color2 },
  { maximize_tt,     0, maximize_restore_cb, 0, 0, FL_NORMAL_LABEL, 0, 14, frame_color2 },
  { restore_tt,      0, maximize_restore_cb, 0, FL_MENU_INACTIVE|FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, 14, frame_color2 },
  { close_tt,        0, close_cb,            0, 0, FL_NORMAL_LABEL, 0, 14, frame_color2 },
  { 0,0,0,0,0,0,0,0,0 }
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
      ret = event;
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

int resize_box::handle(int event)
{
  int ret = Fl_Box::handle(event);
  switch (event)
  {
    case FL_MOVE:
      fl_cursor(FL_CURSOR_SE);
    case FL_PUSH:
      if (tooltips)
      {
        Fl_Tooltip::enable(0);
      }
      x_diff = winw_restore - Fl::event_x();
      y_diff = winh_restore - Fl::event_y();
      ret = event;
      break;
    case FL_DRAG:
      do_callback();
      break;
    case FL_LEAVE:
      fl_cursor(FL_CURSOR_DEFAULT);
    case FL_RELEASE:
      if (tooltips)
      {
        Fl_Tooltip::enable(1);
      }
      break;
  }
  return ret;
}

static void window_restored_settings()
{
  window_maximized = false;
  window_movable = true;
  maximize_icon->show();
  maximize_tooltip_overlay->tooltip(maximize_tt);
  restore_icon_a->hide();
  restore_icon_b->hide();
  b_resize->show();
  b_resize_icon->show();
  mainw->size_range(minw, minh, 0, 0);
  titlebar_menu_items[2].flags = 0;  /* full size */
  titlebar_menu_items[3].flags = FL_MENU_INACTIVE|FL_MENU_DIVIDER;  /* restore */
}

static void window_maximized_settings()
{
  window_maximized = true;
  window_movable = false;
  maximize_icon->hide();
  maximize_tooltip_overlay->tooltip(restore_tt);
  restore_icon_a->show();
  restore_icon_b->show();
  b_resize->hide();
  b_resize_icon->hide();
  mainw->size_range(minw, minh, 0, 0);
  titlebar_menu_items[2].flags = FL_MENU_INACTIVE;  /* full size */
  titlebar_menu_items[3].flags = FL_MENU_DIVIDER;  /* restore */
}

static void move_box_cb(Fl_Widget *)
{
  if (window_movable)
  {
    winx_restore = Fl::event_x_root() - event_x_pos;
    winy_restore = Fl::event_y_root() - event_y_pos;
    mainw->position(winx_restore, winy_restore);
  }
}

static void resize_cb(Fl_Widget *)
{
  if (!window_maximized)
  {
    winw_restore = Fl::event_x() + x_diff;
    winh_restore = Fl::event_y() + y_diff;
    if (winw_restore < minw)
    {
      winw_restore = minw;
    }
    if (winh_restore < minh)
    {
      winh_restore = minh;
    }
    mainw->size_range(minw, minh, (Fl::w() - winx_restore), (Fl::h() - winy_restore));
    mainw->resize(winx_restore, winy_restore, winw_restore, winh_restore);
  }
}

static void iconize_cb(Fl_Widget *, void*)
{
  mainw->iconize();
}

static void default_size_cb(Fl_Widget *, void*)
{
  winw_restore = winw_default;
  winh_restore = winh_default;
  window_restored_settings();
  mainw->resize(winx_restore, winy_restore, winw_restore, winh_restore);
}

static void maximize_restore_cb(Fl_Widget *, void*)
{
  if (window_maximized)
  {
    window_restored_settings();
    mainw->resize(winx_restore, winy_restore, winw_restore, winh_restore);
  }
  else
  {
    window_maximized_settings();
    mainw->resize(0, 0, Fl::w(), Fl::h());
  }
}

static void close_cb(Fl_Widget *, void*)
{
  mainw->hide();
}

static int esc_handler(int event)
{
  if (Fl::event() == FL_SHORTCUT && Fl::event_key() == FL_Escape)
  {
    return 1;
  }
  return 0;
}

static int main_window()
{
  Fl_Double_Window *win;
  Fl_Group *g_titlebar, *g_content, *g_resize;
  Fl_Box *b_titlebar, *b_content, *b_resize_dummy;

  int titlebarh = 24;
  int border = 2;

  Fl_RGB_Image *icon = new Fl_PNG_Image(NULL, icon64_png, icon64_png_len);
#ifdef USE_SIMPLE_MENU_BUTTON
  Fl_RGB_Image *icon24 = new Fl_PNG_Image(NULL, icon24_png, icon24_png_len);
#endif

  Fl_Window::default_icon(icon);

  Fl::add_handler(esc_handler);  /* ignore Escape key */

  if (!tooltips)
  {
    Fl_Tooltip::enable(0);
  }

  winw_restore = winw;
  winh_restore = winh;

  win = mainw = new Fl_Double_Window(winw, winh, win_label);
  {
    /* borders (actually a background) */
    { Fl_Box *o = new Fl_Box(0, 0, winw, winh);
      o->box(FL_FLAT_BOX);
      o->color(frame_color); }

    g_titlebar = new Fl_Group(0, 0, winw, titlebarh + border);
    {
      /* title bar dummy */
      { Fl_Box *o = b_titlebar = new Fl_Box(titlebarh + border*2, border, winw - titlebarh*5 - border*7, titlebarh);
        o->box(FL_FLAT_BOX);
        o->color(frame_color); }

      /* title bar */
      { move_box *o = new move_box(0, border, winw, titlebarh, win_label);
        o->box(FL_NO_BOX);
        o->labelcolor(frame_color2);
        o->callback(move_box_cb); }

      /* title bar menu */
#ifdef USE_SIMPLE_MENU_BUTTON
      { simple_menu_button *o = new simple_menu_button(border, border, titlebarh + border, titlebarh);
        o->image((Fl_Image *)icon24);
#else
      { Fl_Menu_Button *o = new Fl_Menu_Button(border, border, titlebarh + border, titlebarh);
#endif
        o->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
        o->box(FL_FLAT_BOX);
        o->down_box(FL_FLAT_BOX);
        o->color(frame_color);
        o->down_color(frame_color);
        o->clear_visible_focus();
        o->menu(titlebar_menu_items); }

      /* iconize (minimize) button */
      int position = default_size_button ? 4 : 3;
      int bt_iconize_x = winw - (titlebarh*position + border);
      { /* light blue box */
        Fl_Box *o = new Fl_Box(bt_iconize_x + 2, border + 2, titlebarh - 4, titlebarh - 4);
        o->box(FL_FLAT_BOX);
        o->color(bt_color); }
      { /* invisible button with callback */
        Fl_Button *o = new Fl_Button(bt_iconize_x + 2, border + 2, titlebarh - 4, titlebarh - 4);
        o->box(FL_NO_BOX);
        o->clear_visible_focus();
        o->callback(iconize_cb); }
      { /* icon */
        Fl_Box *o = new Fl_Box(bt_iconize_x + 8, border - 4, titlebarh - 16, titlebarh - 16, "_");
        o->box(FL_NO_BOX);
        o->labelfont(FL_HELVETICA);
        o->labelcolor(frame_color2);
        o->labelsize(22); }
      { /* tooltip overlay */
        Fl_Box *o = new Fl_Box(bt_iconize_x + 2, border + 2, titlebarh - 4, titlebarh - 4);
        o->tooltip(iconize_tt);
        o->box(FL_NO_BOX); }

      /* default size button */
      if (default_size_button)
      {
        int bt_default_size_x = winw - (titlebarh*3 + border);
        { /* light blue box */
          Fl_Box *o = new Fl_Box(bt_default_size_x + 2, border + 2, titlebarh - 4, titlebarh - 4);
          o->box(FL_FLAT_BOX);
          o->color(bt_color); }
        { /* invisible button with callback */
          Fl_Button *o = new Fl_Button(bt_default_size_x + 2, border + 2, titlebarh - 4, titlebarh - 4);
          o->box(FL_NO_BOX);
          o->clear_visible_focus();
          o->callback(default_size_cb); }
        { /* icon */
          Fl_Box *o = new Fl_Box(bt_default_size_x + 8, border + 8, titlebarh - 16, titlebarh - 16, "+");
          o->box(FL_NO_BOX);
          o->labelfont(FL_HELVETICA);
          o->labelcolor(frame_color2);
          o->labelsize(14); }
        { /* tooltip overlay */
          Fl_Box *o = new Fl_Box(bt_default_size_x + 2, border + 2, titlebarh - 4, titlebarh - 4);
          o->tooltip(default_size_tt);
          o->box(FL_NO_BOX); }
      }

      /* maximize/restore button */
      int bt_maximize_restore_x = winw - (titlebarh*2 + border);
      { /* light blue box */
        Fl_Box *o = new Fl_Box(bt_maximize_restore_x + 2, border + 2, titlebarh - 4, titlebarh - 4);
        o->box(FL_FLAT_BOX);
        o->color(bt_color); }
      { /* invisible button with callback */
        Fl_Button *o = new Fl_Button(bt_maximize_restore_x + 2, border + 2, titlebarh - 4, titlebarh - 4);
        o->box(FL_NO_BOX);
        o->clear_visible_focus();
        o->callback(maximize_restore_cb); }
      { /* icon */
        { Fl_Box *o = restore_icon_a = new Fl_Box(bt_maximize_restore_x + 10, border + 6, titlebarh - 15, titlebarh - 15);
          o->box(FL_BORDER_FRAME);
          o->color(frame_color2); }
        { Fl_Box *o = new Fl_Box(bt_maximize_restore_x + 6, border + 10, titlebarh - 15, titlebarh - 15);
          o->box(FL_FLAT_BOX);
          o->color(bt_color); }
        { Fl_Box *o = restore_icon_b = new Fl_Box(bt_maximize_restore_x + 6, border + 10, titlebarh - 15, titlebarh - 15);
          o->box(FL_BORDER_FRAME);
          o->color(frame_color2); }
        { Fl_Box *o = maximize_icon = new Fl_Box(bt_maximize_restore_x + 6, border + 6, titlebarh - 12, titlebarh - 12);
          o->box(FL_BORDER_FRAME);
          o->color(frame_color2); }
      }
      { /* tooltip overlay */
        Fl_Box *o = maximize_tooltip_overlay = new Fl_Box(bt_maximize_restore_x + 2, border + 2, titlebarh - 4, titlebarh - 4);
        o->tooltip(maximize_tt);
        o->box(FL_NO_BOX); }

      /* close button */
      int bt_close_x = winw - titlebarh - border;
      { /* light blue box */
        Fl_Box *o = new Fl_Box(bt_close_x + 2, border + 2, titlebarh - 4, titlebarh - 4);
        o->box(FL_FLAT_BOX);
        o->color(fl_lighter(bt_close_color)); }
      { /* invisible button with callback */
        Fl_Button *o = new Fl_Button(bt_close_x + 2, border + 2, titlebarh - 4, titlebarh - 4);
        o->box(FL_NO_BOX);
        o->clear_visible_focus();
        o->callback(close_cb); }
      { /* icon */
        Fl_Box *o = new Fl_Box(bt_close_x + 2, border + 12, titlebarh - 4, 1, "X");
        o->box(FL_NO_BOX);
        o->labelfont(FL_HELVETICA);
        o->labelcolor(frame_color2);
        o->labelsize(16); }
      { /* tooltip overlay */
        Fl_Box *o = new Fl_Box(bt_close_x + 2, border + 2, titlebarh - 4, titlebarh - 4);
        o->tooltip(close_tt);
        o->box(FL_NO_BOX); }
    }
    g_titlebar->end();
    g_titlebar->resizable(b_titlebar);

    g_content = new Fl_Group(border, titlebarh + border*2, winw - border*2, winh - titlebarh - border*3);
    {
      /* gray area */
      { Fl_Box *o = b_content = new Fl_Box(border, titlebarh + border*2, winw - border*2, winh - titlebarh - border*3);
        o->box(FL_FLAT_BOX);
        o->color(main_color); }

      /******************************
      *** add window content here ***
      ******************************/
    }
    g_content->end();
    g_content->resizable(b_content);

    int res_w = 16;
    g_resize = new Fl_Group(winw - res_w*2, winh - res_w*2, res_w*2, res_w*2);
    {
      /* dummy */
      { Fl_Box *o = b_resize_dummy = new Fl_Box(winw - res_w*2, winh - res_w*2, res_w, res_w);
        o->box(FL_NO_BOX); }

      /* resize area */
      { Fl_Box *o = b_resize_icon = new Fl_Box(winw - res_w, winh - res_w, res_w, res_w/2 + 2, ".:");
        o->box(FL_NO_BOX);
        o->labelfont(FL_SYMBOL);
        o->labelcolor(fl_darker(main_color));
        o->labelsize(18); }
      { resize_box *o = b_resize = new resize_box(winw - res_w, winh - res_w, res_w, res_w);
        o->tooltip(resize_tt);
        o->box(FL_NO_BOX);
        o->callback(resize_cb); }
    }
    g_resize->end();
    g_resize->resizable(b_resize_dummy);

    if (window_maximized)
    {
      window_maximized_settings();
    }
    else
    {
      window_restored_settings();
    }
  }
  win->end();

  win->resizable(g_content);
  win->callback(close_cb);

  winx_restore = (Fl::w() - win->w()) / 2;
  winy_restore = (Fl::h() - win->h()) / 2;
  win->position(winx_restore, winy_restore);
  win->size_range(minw, minh, 0, 0);

  Fl::visual(FL_DOUBLE|FL_INDEX);
  win->show();
  win->border(0); /* Use this _after_ show() to remove the WM decoration but keep the taskbar entry */

  return Fl::run();
}

int main(int argc, char **argv)
{
  return main_window();
}

