/*
 *  Copyright (c) 2018, djcj <djcj@gmx.de>
 *
 *  The MediaInfo icon is Copyright (c) 2002-2018, MediaArea.net SARL
 *  All rights reserved.
 *
 *  BSD 2-Clause License
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wshadow"
#endif

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Group.H>
//#include <FL/Fl_Help_View.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Double_Window.H>

/* modified FLTK 1.3.4 header */
#include "Fl_Help_View.H"

#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif

#include <MediaInfo/MediaInfo.h>
#include <ZenLib/Ztring.h>

#include <iostream>
#include <string>
#include <vector>
#include <strings.h>
#include <string.h>

#include "mediainfo.hpp"
#include "icon.h"

#define ICON      MediaInfo_png
#define ICON_LEN  MediaInfo_png_len
#define VENDOR    "https://github.com/darealshinji"
#define APP       "mediainfo-fltk"
#define FL_MENU_DEFAULT  0

static Fl_Double_Window *win, *about_win;
static MyTextDisplay *text;
static MyHelpView *html;
static MyTree *tree;
static const char *view_set = "text";
static int *flags_expand, *flags_collapse;


static void replace_string(const std::string &from, const std::string &to, std::string &s) {
  for (size_t pos = 0; (pos = s.find(from, pos)) != std::string::npos; pos += to.size()) {
    s.replace(pos, from.size(), to);
  }
}

static void do_nothing_cb(Fl_Widget *, void *) {
}

static void tree_expand_all_cb(Fl_Widget *, void *)
{
  Fl_Tree_Item *item = tree->first();

  while (item != tree->last()) {
    item = tree->next_item(item);
    tree->open(item);
  }
}

static void tree_collapse_all_cb(Fl_Widget *, void *)
{
  Fl_Tree_Item *item = tree->first();

  while (item != tree->last()) {
    item = tree->next_item(item);
    tree->close(item);
  }
}

static void load_file(const char *file)
{
  MediaInfoLib::MediaInfo mi;
  ZenLib::Ztring ztr;
  Fl_Text_Buffer *buff;
  std::string str, root;
  std::vector<std::string> vec;
  size_t i, pos;

  if (!file) {
    return;
  }

  ztr = file;

  if (!mi.Open(ztr)) {
    return;
  }

  mi.Option(__T("Output"), __T("HTML"));
  ztr = mi.Inform();
  html->value(ztr.To_Local().c_str());

  mi.Option(__T("Output"), __T("TEXT"));
  ztr = mi.Inform();
  str = ztr.To_Local().c_str();
  buff = text->buffer();
  buff->remove(0, buff->length());  /* clear text buffer */
  buff->text(str.c_str());

  mi.Close();
  ztr.clear();


  /* split string into lines and save them in a vector */

  i = 0;
  pos = str.find('\n');

  while (pos != std::string::npos) {
    vec.push_back(str.substr(i, pos - i));
    i = ++pos;

    if ((pos = str.find('\n', pos)) == std::string::npos) {
      vec.push_back(str.substr(i, str.length()));
    }
  }


  /* tree items */

  tree->clear();
  tree->begin();

  for (const std::string &elem : vec) {
    std::string sub;

    if (elem == "") {
      continue;
    } else if (elem.size() < 44) {
      root = elem;
      continue;
    }

    sub = elem.substr(0, elem.find_last_not_of(' ', 40) + 1);
    replace_string("/", "\xE2\x88\x95", sub);
    str = root + "/" + sub + "/";

    sub = elem.substr(43);
    replace_string("/", "\xE2\x88\x95", sub);
    str += sub;

    tree->add(str.c_str());
  }

  tree->end();
  tree_collapse_all_cb(NULL, NULL);
}

static void dnd_dropped_cb(Fl_Widget *, void *)
{
  char *copy, *p;
  const char *item = Fl::event_text();

  if (strncmp(item, "file:///", 8) != 0) {
    return;
  }

  copy = strdup(item + 7);

  if ((p = strchr(copy, '\n')) == NULL) {
    free(copy);
    return;
  }

  copy[strlen(copy) - strlen(p)] = '\0';
  fl_decode_uri(copy);
  load_file(copy);
  free(copy);
}

static void view_text_cb(Fl_Widget *, void *)
{
  html->hide();
  tree->hide();
  view_set = "text";
  *flags_expand = *flags_collapse = FL_MENU_INACTIVE;

  text->show();
  text->scroll(0, 0);
}

static void view_html_cb(Fl_Widget *, void *)
{
  text->hide();
  tree->hide();
  view_set = "html";
  *flags_expand = *flags_collapse = FL_MENU_INACTIVE;

  html->show();
  html->topline(0);
}

static void view_tree_cb(Fl_Widget *, void *)
{
  text->hide();
  html->hide();
  view_set = "tree";
  *flags_expand = *flags_collapse = FL_MENU_DEFAULT;

  tree->show();
  tree->hposition(0);
  tree->vposition(0);
}

static void open_file_cb(Fl_Widget *, void *)
{
  Fl_Native_File_Chooser *fc = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);
  fc->title("Select a file");

  if (fc->show() == 0) {
    load_file(fc->filename());
  }
}

static void about_cb(Fl_Widget *, void *)
{
  int x = win->x() + win->w()/2 - about_win->w()/2;
  int y = win->y() + win->h()/2 - about_win->h()/2;

  about_win->resize(x, y, about_win->w(), about_win->h());
  about_win->show();
  about_win->take_focus();
}

static void copy_text_selection_cb(Fl_Widget *, void *)
{
  int start, end;
  char *sel;

  Fl_Text_Buffer *buff = text->buffer();
  buff->selection_position(&start, &end);
  sel = buff->text_range(start, end);

  Fl::copy(sel, end - start, 1);
  free(sel);
}

static void copy_html_selection_cb(Fl_Widget *, void *) {
  html->end_selection(1);
}

static void copy_tree_selection_cb(Fl_Widget *, void *)
{
  Fl_Tree_Item *item = tree->last_selected_item();

  if (item) {
    const char *l1, *l2, *l3;

    if (item->depth() == 3) {
      Fl_Tree_Item *par = item->parent();
      l1 = par->parent()->label();
      l2 = item->parent()->label();
      l3 = item->label();
    } else if (item->depth() == 2 && item->has_children()) {
      l1 = item->parent()->label();
      l2 = item->label();
      l3 = item->child(0)->label();
    } else {
      l1 = l2 = l3 = NULL;
    }

    if (l1 && l2 && l3) {
      std::string s = "[" + std::string(l1) + "] " + std::string(l2) + ": " + std::string(l3);
      replace_string("\xE2\x88\x95", "/", s);
      Fl::copy(s.c_str(), s.size(), 1);
    }
  }
}

static void close_cb(Fl_Widget *, void *v)
{
  about_win->hide();
  win->hide();

  if (v) {
    Fl_Preferences *pref = reinterpret_cast<Fl_Preferences *>(v);
    pref->set("view", view_set);
    pref->flush();
  }
}

int main(int argc, char *argv[])
{
  Fl_Group *g, *g_inside, *g_top;
  Fl_Preferences *pref = NULL;
  Fl_Text_Buffer *buff;
  char *home, view_get[8] = {0};
  int *flags_text, *flags_html, *flags_tree;

  if ((home = getenv("HOME")) != NULL) {
    std::string s = std::string(home) + "/.config";
    pref = new Fl_Preferences(s.c_str(), VENDOR, APP);
    if (!pref->get("view", view_get, "text", sizeof(view_get) - 1)) {
      memset(view_get, '\0', sizeof(view_get));
    }
  }

  Fl_Menu_Item menu[] = {
    { "File", 0, NULL, NULL, FL_SUBMENU },
      { " Open file  ", 0, open_file_cb, NULL, FL_MENU_DIVIDER },
      { " Close window  ", 0, close_cb, pref },
      {0},
    { "View", 0, NULL, NULL, FL_SUBMENU },
      { " Text  ", 0, view_text_cb, NULL, FL_MENU_RADIO },
      { " HTML  ", 0, view_html_cb, NULL, FL_MENU_RADIO },
      { " Tree  ", 0, view_tree_cb, NULL, FL_MENU_RADIO|FL_MENU_DIVIDER },
      { " Expand all  ", 0, tree_expand_all_cb, NULL, FL_MENU_INACTIVE },
      { " Collapse all  ", 0, tree_collapse_all_cb, NULL, FL_MENU_INACTIVE },
      {0},
    { "Help", 0, NULL, NULL, FL_SUBMENU },
      { " About  ", 0, about_cb },
      {0},
    {0}
  };

  const int i = 5;
  flags_text = &menu[i].flags;
  flags_html = &menu[i + 1].flags;
  flags_tree = &menu[i + 2].flags;
  flags_expand = &menu[i + 3].flags;
  flags_collapse = &menu[i + 4].flags;

  Fl_Menu_Item text_menu[] = {
    { " Copy selection  ", 0, copy_text_selection_cb, NULL, FL_MENU_DIVIDER },
    { " Dismiss  ", 0, do_nothing_cb },
    {0}
  };

  Fl_Menu_Item html_menu[] = {
    { " Copy selection  ", 0, copy_html_selection_cb, NULL, FL_MENU_DIVIDER },
    { " Dismiss  ", 0, do_nothing_cb },
    {0}
  };

  Fl_Menu_Item tree_menu[] = {
    { " Copy selection  ", 0, copy_tree_selection_cb },
    { " Expand all  ", 0, tree_expand_all_cb },
    { " Collapse all  ", 0, tree_collapse_all_cb, NULL, FL_MENU_DIVIDER },
    { " Dismiss  ", 0, do_nothing_cb },
    {0}
  };

  /* http://fltk.org/str.php?L3465+P0+S-2+C0+I0+E0+V%25+QFL_SCREEN */
  Fl::set_font(FL_SCREEN, " mono");

  Fl_Window::default_icon(new Fl_PNG_Image(NULL, ICON, ICON_LEN));

  win = new Fl_Double_Window(800, 600, "MediaInfo");
  win->callback(close_cb, pref);
  {
    g = new Fl_Group(0, 0, 800, 600);
    {
      g_top = new Fl_Group(10, 0, 780, 30);
      {
        Fl_Menu_Bar *o = new Fl_Menu_Bar(10, 0, 200, 30);
        o->box(FL_NO_BOX);
        o->menu(menu);
      }
      g_top->end();
      g_top->resizable(NULL);

      g_inside = new Fl_Group(10, 30, 780, 560);
      {
        text = new MyTextDisplay(10, 30, 780, 560);
        buff = new Fl_Text_Buffer();
        text->buffer(buff);
        text->textfont(FL_SCREEN);  // FL_COURIER
        text->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 1);
        text->menu(text_menu);

        html = new MyHelpView(10, 30, 780, 560);
        html->menu(html_menu);
        html->hide();

        tree = new MyTree(10, 30, 780, 560);
        tree->showroot(0);
        tree->menu(tree_menu);
        tree->hide();
      }
      g_inside->end();

      { MyDndBox *o = new MyDndBox(10, 30, 780, 560);
       o->callback(dnd_dropped_cb); }
    }
    g->end();
    g->resizable(g_inside);
  }
  win->end();
  win->resizable(g);
  win->position((Fl::w() - 800) / 2, (Fl::h() - 600) / 2); /* center */
  win->show();

  about_win = new Fl_Double_Window(260, 130, "About");
  {
    MediaInfoLib::MediaInfo mi;

    ZenLib::Ztring url = mi.Option(__T("Info_Url"));
    ZenLib::Ztring ztr =
      __T("<html><body bgcolor=silver><center><p></p><p>Using ")
      + mi.Option(__T("Info_Version"))
      + __T("</p><p></p><p><a href=\"") + url + __T("\">")
      + url + __T("</a></p></center></body></html>");

    { Fl_Help_View *o = new Fl_Help_View(0, 0, 260, 130);
     o->box(FL_FLAT_BOX);
     o->value(ztr.To_Local().c_str()); }
  }
  about_win->end();

  if (strcasecmp(view_get, "html") == 0) {
    *flags_html = FL_MENU_RADIO|FL_MENU_VALUE;
    view_html_cb(NULL, NULL);
  } else if (strcasecmp(view_get, "tree") == 0) {
    *flags_tree = FL_MENU_RADIO|FL_MENU_VALUE|FL_MENU_DIVIDER;
    *flags_expand = *flags_collapse = FL_MENU_DEFAULT;
    view_tree_cb(NULL, NULL);
  } else {
    *flags_text = FL_MENU_RADIO|FL_MENU_VALUE;
    //view_text_cb(NULL, NULL);
  }

  if (argc > 1) {
    load_file(argv[1]);
  }

  return Fl::run();
}

