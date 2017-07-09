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

/* edited version of Fl_Menu_Button.H
 * 2016-12-7 djcj <djcj@gmx.de>
 */

#ifndef simple_menu_button_H
#define simple_menu_button_H

#include <FL/Fl_Menu_.H>
#include <FL/fl_draw.H>

class simple_menu_button : public Fl_Menu_ {
protected:
  void draw();
public:
  enum popup_buttons {POPUP1 = 1, /**< pops up with the mouse 1st button. */
    POPUP2,  /**< pops up with the mouse 2nd button. */
    POPUP12, /**< pops up with the mouse 1st or 2nd buttons. */
    POPUP3,   /**< pops up with the mouse 3rd button. */
    POPUP13,  /**< pops up with the mouse 1st or 3rd buttons. */
    POPUP23,  /**< pops up with the mouse 2nd or 3rd buttons. */
    POPUP123 /**< pops up with any mouse button. */
  };
  int handle(int);
  const Fl_Menu_Item* popup();
  simple_menu_button(int X, int Y, int W, int H, const char *L=0) : Fl_Menu_(X, Y, W, H, L) { }
};

#endif

