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

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Help_View.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Double_Window.H>

#include <MediaInfo/MediaInfo.h>
#include <ZenLib/Ztring.h>

#include <iostream>
#include <string>
#include <vector>
#include <strings.h>
#include <string.h>

static void dnd_dropped(const char *item);

class dnd_box : public Fl_Box
{
public:
  dnd_box(int X, int Y, int W, int H)
   : Fl_Box(X, Y, W, H) { }

  int handle(int event) {
    switch (event) {
      case FL_DND_ENTER:
      case FL_DND_DRAG:
      case FL_DND_RELEASE:
        return 1;
      case FL_PASTE:
        dnd_dropped(Fl::event_text());
        return 1;
    }
    return Fl_Box::handle(event);
  }
};

static const unsigned char png_buff[] = {
  0x89,0x50,0x4e,0x47,0xd,0xa,0x1a,0xa,0x0,0x0,0x0,0xd,0x49,0x48,0x44,0x52,0x0,0x0,0x0,0x40,0x0,0x0,0x0,0x40,0x8,0x3,
  0x0,0x0,0x0,0x9d,0xb7,0x81,0xec,0x0,0x0,0x1,0x23,0x50,0x4c,0x54,0x45,0x0,0x0,0x0,0xf,0x30,0xd7,0x7d,0x6d,0xd,0x12,
  0x10,0xfe,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xff,0x3,0x2,
  0xff,0x3,0x2,0x0,0x0,0x0,0x0,0x0,0x0,0xff,0x2,0x2,0x0,0x0,0x0,0x12,0xf,0xfe,0x12,0xf,0xfe,0x12,0xf,0xfe,0x12,0xf,
  0xfe,0x13,0x10,0xff,0x13,0x10,0xff,0x0,0x0,0x0,0x1,0xd3,0x18,0xff,0x3,0x2,0x12,0xf,0xfe,0xff,0xff,0xff,0xd,0xd,0xd,
  0x5e,0x5e,0x5e,0x58,0x58,0x58,0x2c,0x2c,0x2c,0xee,0xee,0xee,0xaf,0xaf,0xaf,0xa2,0xa2,0xa2,0x52,0x52,0x52,0x4c,0x4c,
  0x4c,0x41,0x41,0x41,0xf8,0xf8,0xf8,0xe6,0xe6,0xe6,0xd9,0xd9,0xd9,0xbb,0xbb,0xbb,0x9b,0x9b,0x9b,0x88,0x88,0x88,0x78,
  0x78,0x78,0x70,0x70,0x70,0x67,0x67,0x67,0x14,0x14,0x14,0x50,0x6a,0xd9,0x6e,0x0,0x0,0x0,0x48,0x74,0x52,0x4e,0x53,0x0,
  0xde,0xc2,0xe6,0x7,0xd,0x3,0xe3,0x29,0xfb,0xf8,0xf5,0x78,0x46,0x1e,0xfd,0xed,0x73,0x4e,0x24,0x3c,0xf1,0xa1,0x8d,0x32,
  0xe9,0xde,0xdb,0xb5,0x98,0x87,0x54,0x36,0x10,0x4,0xd6,0xd2,0xc2,0xad,0x9d,0x58,0x4a,0xa8,0x8f,0x8a,0x41,0x38,0x2e,
  0xc5,0x67,0x62,0x1b,0x18,0x13,0xdf,0xc6,0xbc,0x93,0x81,0x6d,0xf5,0xed,0xd0,0x91,0x68,0x5c,0xf0,0xef,0xd3,0xd2,0x51,
  0x50,0xae,0x7d,0xf4,0x58,0x0,0x0,0x3,0x7,0x49,0x44,0x41,0x54,0x58,0xc3,0xc5,0x97,0xe7,0x7a,0xda,0x30,0x14,0x86,0xd5,
  0xd6,0x36,0x6,0x6c,0xb3,0x37,0x21,0x4,0x8,0x9b,0x2c,0x32,0x9b,0xb6,0x69,0x3a,0xbf,0x24,0xce,0x1e,0xdd,0xe3,0xfe,0xaf,
  0xa2,0x8f,0x14,0x19,0x99,0x61,0x62,0x8b,0x1f,0x7d,0x7f,0x21,0x1e,0xf4,0xa2,0x71,0xce,0x91,0x44,0xde,0xbe,0x3a,0x7d,
  0x7e,0x72,0x72,0xf2,0xec,0xec,0xfd,0x67,0x32,0x81,0xa6,0xa8,0x96,0x9e,0x4f,0xa6,0x15,0x32,0x8f,0xd7,0xa7,0x5c,0x70,
  0xf6,0xd1,0xe3,0x17,0x79,0x23,0xdb,0x33,0x6b,0x9e,0x82,0xd3,0x91,0xe0,0x5,0xf1,0x64,0xd0,0x8d,0x67,0x4d,0x4d,0x42,
  0x20,0xd0,0x9b,0x28,0x25,0x76,0x24,0x4,0x82,0x74,0x1d,0xd1,0xca,0xca,0x2,0x2,0xa2,0xe4,0x80,0x78,0xa2,0x26,0x25,0xe0,
  0x24,0x33,0x40,0xa3,0xba,0x80,0x80,0xe8,0x21,0x20,0x56,0xa9,0xc9,0xb,0x88,0x55,0x2,0xb0,0xba,0x2d,0x2f,0x20,0xc3,
  0x22,0x80,0x50,0x5e,0x5e,0x40,0xf4,0x38,0x80,0xe8,0x40,0x5e,0x40,0x6,0x61,0x6a,0x30,0xe5,0x5,0xc4,0x0,0x35,0xac,0xf9,
  0x12,0x28,0x7a,0x3b,0xd7,0xec,0x27,0xc7,0xc3,0x47,0x5b,0xa5,0x86,0x42,0xca,0xef,0x8,0x86,0xfd,0x68,0xb8,0xbc,0xec,
  0x4e,0x84,0x54,0x84,0x1a,0x1a,0x8a,0xef,0x29,0xc,0xcb,0x40,0xa9,0x4d,0x4,0x15,0x50,0x8e,0x2,0xac,0x41,0x22,0x6,0xec,
  0x5b,0xa3,0xa6,0x5a,0x60,0x86,0x4e,0x80,0x45,0xdc,0x4,0x10,0x37,0x85,0x90,0x9,0x8a,0x4a,0x80,0x5d,0x38,0x4,0x10,0xde,
  0x1a,0xd,0x21,0xc3,0xc,0x46,0x90,0x6d,0x5c,0x2,0x10,0x1b,0x2d,0x44,0x8e,0x9,0x32,0x3b,0x1,0x4,0x6a,0x88,0x8e,0x21,
  0xcd,0x5b,0xeb,0x60,0x18,0x41,0x2,0xc9,0xa4,0x3d,0x42,0x2a,0x6f,0xd5,0xc1,0x9a,0x35,0x6f,0x81,0x92,0x5a,0xdf,0x1e,
  0x37,0xb0,0xf8,0x39,0xe0,0x8d,0x97,0x60,0x24,0xbd,0x4,0xcb,0xe5,0x8,0xfd,0x83,0x3,0xcb,0xfd,0x1d,0xed,0x11,0xd3,0x79,
  0x81,0x3,0x23,0x3b,0x5b,0x60,0x6d,0x80,0x13,0x31,0x84,0x40,0x2b,0xf2,0x2e,0x14,0x25,0x2,0x4a,0x5c,0x9b,0x16,0xb0,
  0xba,0x23,0x68,0x9,0x43,0x8e,0x29,0x57,0x9c,0x9,0x31,0xd6,0x66,0x8,0xd4,0x3a,0xdc,0x88,0x31,0x74,0x58,0x7b,0xcb,0xd9,
  0x56,0xc6,0xe1,0xc,0x41,0xf,0x94,0xf3,0x87,0xbb,0x2b,0x50,0xc2,0xa9,0xd1,0x4e,0x86,0x69,0xbb,0xeb,0xe,0x46,0x2c,0x4d,
  0xb,0xb6,0xd9,0xec,0xee,0xbf,0xda,0xb6,0x7d,0x73,0xce,0x67,0xcd,0x79,0xc3,0xb6,0x8e,0x57,0x68,0x30,0xf6,0xdc,0x2,
  0x11,0xf6,0xf8,0x7b,0x7d,0x7d,0x77,0x75,0x6b,0xff,0x62,0xf1,0xa6,0x38,0x82,0x26,0x28,0xaa,0x7b,0x1b,0x8a,0xd3,0x82,
  0x2e,0xfd,0xfe,0xea,0xe7,0x25,0xf0,0x60,0xff,0xe0,0xb,0x45,0xdc,0xf3,0xd6,0xdd,0xb1,0x18,0x9f,0x16,0x8c,0xb6,0xf0,
  0xfc,0xc6,0xbe,0x4,0x65,0xd9,0x11,0xf4,0x59,0xb3,0xfa,0x58,0x55,0xc0,0x8,0x4f,0xb,0x1a,0x4e,0xff,0xef,0x5f,0x7e,0x83,
  0xaf,0x3b,0x27,0xe7,0x4b,0xb0,0xcf,0x5,0xb7,0xf6,0x3d,0xff,0xd4,0x9,0x36,0x85,0x3,0xde,0xed,0xe2,0x2,0x9c,0x54,0xb0,
  0x45,0xec,0x80,0x11,0xfb,0xe3,0x8,0xea,0xf3,0xb6,0x71,0x63,0x5a,0x50,0x2b,0x3d,0x2e,0x81,0xfd,0x8d,0xb,0x12,0x84,
  0xb3,0x12,0xf3,0x15,0x48,0xa4,0xfd,0x38,0x82,0x8b,0x4b,0x30,0x76,0x95,0xa0,0xa1,0x4c,0x5a,0x70,0x11,0xa9,0x8e,0x27,
  0x53,0xf4,0xe9,0x64,0x22,0x4a,0x56,0xf4,0xcf,0x98,0x22,0x9d,0x77,0x9d,0xec,0x14,0xe9,0x5c,0x98,0x4e,0x67,0xc6,0x66,
  0x81,0xf7,0x2f,0xa7,0x26,0xb,0x4a,0x6a,0xac,0xa0,0xb4,0xbc,0x2a,0x92,0xda,0xce,0x6e,0x34,0xca,0x95,0x3c,0x71,0xb1,
  0x47,0x7b,0xf4,0x7c,0x94,0x34,0xf,0x3a,0x6c,0x45,0x27,0x8a,0xaa,0x16,0xb0,0xac,0x47,0x9c,0x21,0xe5,0x83,0x94,0x75,
  0xb1,0x35,0xb1,0x51,0x5a,0xf5,0x2,0x1f,0x2c,0x9,0x9a,0x38,0xc7,0x13,0x47,0x5b,0x42,0xfa,0x70,0x35,0x82,0x1e,0xae,0x6,
  0x3d,0xde,0x87,0x93,0xc7,0xbb,0x49,0x7c,0xa,0xac,0x32,0x50,0x77,0xa6,0x2f,0xa,0x4b,0x8f,0xf8,0x13,0x58,0xb9,0x68,
  0xa4,0x9c,0x94,0xbc,0xe2,0x28,0xfa,0x71,0xae,0x59,0x99,0xb8,0x64,0xd5,0x58,0x44,0x15,0x2c,0x99,0x6b,0x9e,0x8,0xc2,
  0x68,0x95,0xc8,0xa,0x4c,0x71,0xd1,0x94,0xbf,0xea,0x66,0xd2,0x44,0x56,0x60,0xf1,0xcb,0xf6,0xff,0xba,0xee,0xaf,0x87,
  0x80,0x70,0x45,0x23,0xb2,0x82,0x76,0x74,0xa1,0x27,0x8f,0x7a,0xb4,0xd8,0xa3,0x2b,0x5d,0x5a,0xe8,0xd9,0xa7,0x77,0x51,
  0x32,0xe4,0x1f,0x9e,0x83,0x66,0xa1,0x65,0x3e,0xf9,0xf8,0xfe,0x40,0x66,0xa2,0x55,0x8d,0x6c,0x7f,0xa0,0x79,0xb9,0xc5,
  0xf3,0xff,0xdd,0x27,0x8f,0xe7,0xff,0x9a,0x42,0xe6,0xf0,0xf,0x1a,0x84,0x6f,0xf,0x63,0x10,0x29,0xe7,0x0,0x0,0x0,0x0,
  0x49,0x45,0x4e,0x44,0xae,0x42,0x60,0x82
};

#define VENDOR  "https://github.com/darealshinji"
#define APP     "mediainfo-fltk"

static Fl_Double_Window *win, *about_win;
static Fl_Text_Buffer *buff;
static Fl_Text_Display *text;
static Fl_Help_View *html;
static Fl_Tree *tree;
static Fl_Preferences *prefs = NULL;
static MediaInfoLib::MediaInfo mi;

static const char *view_set = "text";
static const int ABOUT_W = 260, ABOUT_H = 130;


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

/* replace regular slash with division slash */
static void division_slash(std::string &str) {
  for (size_t pos = 0; (pos = str.find('/', pos)) != std::string::npos; pos += 3) {
    str.replace(pos, 1, "\xE2\x88\x95");
  }
}

static void load_file(const char *file)
{
  ZenLib::Ztring ztr;
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
    division_slash(sub);
    str = root + "/" + sub + "/";

    sub = elem.substr(43);
    division_slash(sub);
    str += sub;

    tree->add(str.c_str());
  }

  tree->end();
  tree_collapse_all_cb(NULL, NULL);
}

static void dnd_dropped(const char *item)
{
  char *copy, *p;

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
  text->show();
  text->scroll(0, 0);
  html->hide();
  tree->hide();
  view_set = "text";
}

static void view_html_cb(Fl_Widget *, void *)
{
  text->hide();
  html->show();
  html->topline(0);
  tree->hide();
  view_set = "html";
}

static void view_tree_cb(Fl_Widget *, void *)
{
  text->hide();
  html->hide();
  tree->show();
  tree->hposition(0);
  tree->vposition(0);
  view_set = "tree";
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
  int x = win->x() + win->w()/2 - ABOUT_W/2;
  int y = win->y() + win->h()/2 - ABOUT_H/2;

  about_win->resize(x, y, ABOUT_W, ABOUT_H);
  about_win->show();
  about_win->take_focus();
}

static void close_cb(Fl_Widget *, void *)
{
  about_win->hide();
  win->hide();

  if (prefs) {
    prefs->set("view", view_set);
    prefs->flush();
  }
}

int main(int argc, char *argv[])
{
  Fl_Group *g, *g_inside, *g_top;
  Fl_Menu_Bar *menu_bar;
  ZenLib::Ztring ver, url;
  std::string str;
  char *home;
  char view_get[8] = {0};
  int *flags_text, *flags_html, *flags_tree;

  Fl_Menu_Item menu[] = {
    { "File", 0,0,0, FL_SUBMENU },
      { " Open file  ", 0, open_file_cb, 0, FL_MENU_DIVIDER },
      { " Close window  ", 0, close_cb },
      {0},
    { "View", 0,0,0, FL_SUBMENU },
      { " Text  ", 0, view_text_cb, 0, FL_MENU_RADIO },
      { " HTML  ", 0, view_html_cb, 0, FL_MENU_RADIO },
      { " Tree  ", 0, view_tree_cb, 0, FL_MENU_RADIO|FL_MENU_DIVIDER },
      { " Expand all (tree view)  ", 0, tree_expand_all_cb },
      { " Collapse all (tree view)  ", 0, tree_collapse_all_cb },
      {0},
    { "Help", 0,0,0, FL_SUBMENU },
      { " About  ", 0, about_cb },
      {0},
    {0}
  };

  flags_text = &menu[5].flags;
  flags_html = &menu[6].flags;
  flags_tree = &menu[7].flags;

  if ((home = getenv("HOME")) != NULL) {
    str = std::string(home) + "/.config";
    prefs = new Fl_Preferences(str.c_str(), VENDOR, APP);
    if (!prefs->get("view", view_get, "text", sizeof(view_get))) {
      view_get[0] = '\0';
    }
  }

  /* http://fltk.org/str.php?L3465+P0+S-2+C0+I0+E0+V%25+QFL_SCREEN */
  Fl::set_font(FL_SCREEN, " mono");

  Fl_Window::default_icon(new Fl_PNG_Image(NULL, png_buff, sizeof(png_buff)));

  win = new Fl_Double_Window(800, 600, "MediaInfo");
  win->callback(close_cb, NULL);
  {
    g = new Fl_Group(0, 0, 800, 600);
    {
      g_top = new Fl_Group(10, 0, 780, 30);
      {
        menu_bar = new Fl_Menu_Bar(10, 0, 200, 30);
        menu_bar->box(FL_NO_BOX);
        menu_bar->menu(menu);
      }
      g_top->end();
      g_top->resizable(NULL);

      g_inside = new Fl_Group(10, 30, 780, 560);
      {
        text = new Fl_Text_Display(10, 30, 780, 560);
        buff = new Fl_Text_Buffer();
        text->buffer(buff);
        text->textfont(FL_SCREEN);  // FL_COURIER
        text->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 1);
        //text->hide();

        html = new Fl_Help_View(10, 30, 780, 560);
        html->hide();

        tree = new Fl_Tree(10, 30, 780, 560);
        tree->showroot(0);
        tree->hide();
      }
      g_inside->end();

      new dnd_box(10, 30, 780, 560);
    }
    g->end();
    g->resizable(g_inside);
  }
  win->end();
  win->resizable(g);
  win->position((Fl::w() - 800) / 2, (Fl::h() - 600) / 2); /* center */
  win->show();

  about_win = new Fl_Double_Window(ABOUT_W, ABOUT_H, "About");
  {
    ver = mi.Option(__T("Info_Version"));
    url = mi.Option(__T("Info_Url"));
    str = "<body bgcolor=silver><center><p></p><p>Using ";
    str += ver.To_Local().c_str();
    str += "</p><p></p><p><a href=\"";
    str += url.To_Local().c_str();
    str += "\">";
    str += url.To_Local().c_str();
    str += "</a></p></center></body>";

    Fl_Help_View *o = new Fl_Help_View(0, 0, ABOUT_W, ABOUT_H);
    o->box(FL_FLAT_BOX);
    o->value(str.c_str());

    ver.clear();
    url.clear();
    str.clear();
  }
  about_win->end();

  if (strcasecmp(view_get, "html") == 0) {
    *flags_html = FL_MENU_RADIO|FL_MENU_VALUE;
    view_html_cb(NULL, NULL);
  } else if (strcasecmp(view_get, "tree") == 0) {
    *flags_tree = FL_MENU_RADIO|FL_MENU_VALUE|FL_MENU_DIVIDER;
    view_tree_cb(NULL, NULL);
  } else {
    *flags_text = FL_MENU_RADIO|FL_MENU_VALUE;
  }

  if (argc > 1) {
    load_file(argv[1]);
  }

  return Fl::run();
}
