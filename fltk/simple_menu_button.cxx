//
// Menu button header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

/* edited version of Fl_Menu_Button.cxx:
 * arrow symbol removed and functions renamed
 * 2016-12-7 djcj <djcj@gmx.de>
 */

#include "simple_menu_button.H"


static simple_menu_button	*pressed_menu_button_ = 0;

void simple_menu_button::draw() {
  if (!box() || type()) return;
  int H = (labelsize()-3)&-2;
  int X = x()+w()-H-Fl::box_dx(box())-Fl::box_dw(box())-1;
  int Y = y()+(h()-H)/2;
  draw_box(pressed_menu_button_ == this ? fl_down(box()) : box(), color());
  draw_label(x()+Fl::box_dx(box()), y(), X-x()+2, h());
  if (Fl::focus() == this) draw_focus();
}

const Fl_Menu_Item *simple_menu_button::popup() {
  const Fl_Menu_Item *m;
  pressed_menu_button_ = this;
  redraw();
  Fl_Widget_Tracker mb(this);
  if (!box() || type()) {
    m = menu()->popup(Fl::event_x(), Fl::event_y(), label(), mvalue(), this);
  } else {
    m = menu()->pulldown(x(), y(), w(), h(), 0, this);
  }
  picked(m);
  pressed_menu_button_ = 0;
  if (mb.exists()) redraw();
  return m;
}

int simple_menu_button::handle(int e) {
  if (!menu() || !menu()->text) return 0;
  switch (e) {
  case FL_ENTER: /* FALLTHROUGH */
  case FL_LEAVE:
    return (box() && !type()) ? 1 : 0;
  case FL_PUSH:
    if (!box()) {
      if (Fl::event_button() != 3) return 0;
    } else if (type()) {
      if (!(type() & (1 << (Fl::event_button()-1)))) return 0;
    }
    if (Fl::visible_focus()) Fl::focus(this);
    popup();
    return 1;
  case FL_KEYBOARD:
    if (!box()) return 0;
    if (Fl::event_key() == ' ' &&
        !(Fl::event_state() & (FL_SHIFT | FL_CTRL | FL_ALT | FL_META))) {
      popup();
      return 1;
    } else return 0;
  case FL_SHORTCUT:
    if (Fl_Widget::test_shortcut()) {popup(); return 1;}
    return test_shortcut() != 0;
  case FL_FOCUS: /* FALLTHROUGH */
  case FL_UNFOCUS:
    if (box() && Fl::visible_focus()) {
      redraw();
      return 1;
    }
  default:
    return 0;
  }
}

