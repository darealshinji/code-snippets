/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017-2018, djcj <djcj@gmx.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define WITH_ICON

#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Multi_Browser.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Double_Window.H>
#include <FL/filename.H>
#ifdef WITH_ICON
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_RGB_Image.H>
#endif

#include <sstream>
#include <iostream>
#include <iomanip>
#include <ios>
#include <string>
#include <vector>

#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <zlib.h>

/**
 * Icon source:
 * https://cgit.haiku-os.org/haiku/tree/data/artwork/icons/App_DOS-Box
 * Copyright (c) 2009,  Haiku, Inc.
 * The MIT License (MIT)
 */
#ifdef WITH_ICON
#include "crc32_check.xpm"
#endif

#ifndef nullptr
void *_null_ptr = NULL;
#define NULLPTR _null_ptr
#else
#define NULLPTR nullptr
#endif

class dnd_box : public Fl_Box
{
public:
  dnd_box(int X, int Y, int W, int H)
    : Fl_Box(X, Y, W, H) { }

  virtual ~dnd_box() { }

  int handle(int event);
};

Fl_Double_Window *win;
Fl_Multi_Browser *browser;
Fl_Button *bt_copy, *bt_add;
dnd_box *box;

#define MAX_ENTRIES 1000
std::string list[MAX_ENTRIES + 1];
std::string list_bn[MAX_ENTRIES + 1];
std::string list_crc[MAX_ENTRIES + 1];
int current_line = 0;
int itemcount = 0;
bool max_reached = false;
bool odd = true;

#define WARNING "Maximum file entries reached!"

void dnd_callback(const char *items);
void split(const std::string &s, char c, std::vector<std::string> &v);
long calculate_crc32(const char *file);

int dnd_box::handle(int event)
{
  int ret = Fl_Box::handle(event);
  switch (event) {
    case FL_DND_ENTER:
    case FL_DND_DRAG:
    case FL_DND_RELEASE:
      ret = 1;
      break;
    case FL_PASTE: {
      if (max_reached) {
        fl_alert(WARNING);
      } else {
        dnd_callback(Fl::event_text());
      }
      ret = 1;
      break;
    }
  }
  return ret;
}

void split(const std::string &s, char c, std::vector<std::string> &v)
{
  size_t i = 0;
  size_t j = s.find(c);
  while (j != std::string::npos) {
    v.push_back(s.substr(i, j - i));
    i = ++j;
    j = s.find(c, j);
    if (j == std::string::npos) {
      v.push_back(s.substr(i, s.length()));
    }
  }
}

void dnd_callback(const char *items)
{
  if (!max_reached && strncmp(items, "file:///", 8) == 0 && strlen(items) > 8) {
    std::vector<std::string> vector;
    split(std::string(items), '\n', vector);

    for (size_t i = 0; i < vector.size(); i++) {
      char *line = strdup(vector[i].c_str() + 7);
      fl_decode_uri(line);

      if (access(line, R_OK) == 0) {
        itemcount++;
        if (itemcount <= MAX_ENTRIES) {
          list[itemcount] = std::string(line);
          char *bn = basename(line);
          list_bn[itemcount] = bn ? std::string(bn) : "";
          std::string s = "\t@." + list_bn[itemcount];
          browser->add(s.c_str());
          win->redraw();
        }
      }
      free(line);
    }
  }
}

extern "C" void *get_crc_checksum(void *)
{
  while (true) {
    if (!max_reached && current_line < itemcount) {
      current_line++;

      Fl::lock();
      browser->bottomline(current_line);
      Fl::unlock();
      Fl::awake(win);

      long crc = calculate_crc32(list[current_line].c_str());
      std::string bg = odd ? "@B255" : "@B17";  /* white / light yellow */
      std::string entry = bg + "@f@c";
      odd = odd ? false : true;

      if (crc == -1) {
        entry += "88@.ERROR";  /* red */
      } else {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(8) << std::hex << std::uppercase << crc;
        list_crc[current_line] = ss.str();

        if (strcasestr(browser->text(current_line), list_crc[current_line].c_str())) {
          entry += "@C60";  /* green */
        } else {
          entry += "@C88";  /* red */
        }
        entry += "@." + list_crc[current_line];
      }
      entry += "\t" + bg + "@." + list_bn[current_line];

      Fl::lock();
      browser->remove(current_line);
      browser->insert(current_line, entry.c_str());
      if (current_line == MAX_ENTRIES) {
        max_reached = true;
        bt_add->deactivate();
        fl_alert(WARNING);
      }
      Fl::unlock();
      Fl::awake(win);
    }

    usleep(100); /* needed, somehow... */
  }

  return NULLPTR;
}

long calculate_crc32(const char *file)
{
  FILE *fp;
  size_t items;
  Bytef buf[262144]; /* 256k */
  long byteCount = 0L;
  uInt completed = 0;

  if (!(fp = fopen(file, "r"))) {
    return -1;
  }

  if (fseek(fp, 0, SEEK_END) == -1) {
    fclose(fp);
    return -1;
  }

  long fileSize = ftell(fp);
  rewind(fp);

  uLong crc = crc32(0, NULL, 0);

  while (feof(fp) == 0) {
    items = fread(buf, sizeof(*buf), sizeof(buf)/sizeof(*buf), fp);

    if (ferror(fp) != 0) {
      if (file) {
        fclose(fp);
      }
      return -1;
    }

    if (file) {
      byteCount += (long)(items * sizeof(*buf));
      uInt n = (uInt)((float)byteCount/(float)fileSize*100.0);

      if (n > completed && n < 100) {
        std::stringstream ss;
        completed = n;
        ss << completed;
        std::string s = "@f@c" + ss.str() + "%\t@." + list_bn[current_line];

        Fl::lock();
        browser->remove(current_line);
        browser->insert(current_line, s.c_str());
        Fl::unlock();
        Fl::awake(win);
      }
    }

    crc = crc32(crc, (const Bytef *)buf, (uInt)(items * sizeof(*buf)));
  }

  return (long)crc;
}

void browser_cb(Fl_Widget *) {
  if (itemcount > 0 && !bt_copy->active()) {
    bt_copy->activate();
  }
}

void add_cb(Fl_Widget *)
{
  Fl_Native_File_Chooser gtk;
  std::string entry;
  const char *file = NULL, *title = "Select a file";

  if (getenv("KDE_FULL_SESSION")) {
    /* don't use GTK file chooser on KDE, there may be layout issues */
    file = fl_file_chooser(title, "*", NULL);
  } else {
    gtk.title(title);
    gtk.type(Fl_Native_File_Chooser::BROWSE_FILE);
    if (gtk.show() == 0) {
      file = gtk.filename();
    }
  }

  if (!file) {
    return;
  }

  itemcount++;
  list[itemcount] = std::string(file);
  char *bn = basename((char *)file);
  list_bn[itemcount] = bn ? std::string(bn) : "";
  entry = "\t@." + list_bn[itemcount];
  browser->add(entry.c_str());
  win->redraw();
}

void copy_cb(Fl_Widget *) {
  const char *text = list_crc[browser->value()].c_str();
  int len = strlen(text);
  if (len == 8) {
    Fl::copy(text, len, 1, Fl::clipboard_plain_text);
  }
}

void close_cb(Fl_Widget *) {
  win->hide();
}

int main(void)
{
  Fl_Button *bt_close;
  Fl_Box *dummy;
  Fl_Group *g;
  int winw = 640, winh = 400, butw = 90;

  /* satisfying section 4 of the FLTK license's LGPL exception */
  std::cout << "FLTK version " << FL_MAJOR_VERSION << "." << FL_MINOR_VERSION
    << "." << FL_PATCH_VERSION << " (http://www.fltk.org)\n"
    << "zlib version " ZLIB_VERSION << " (https://zlib.net)"
    << std::endl;

#ifdef WITH_ICON
  Fl_Pixmap pixmap(icon_xpm);
  Fl_RGB_Image icon(&pixmap, Fl_Color(0));
  Fl_Window::default_icon(&icon);
#endif
  Fl::visual(FL_DOUBLE|FL_INDEX);
  fl_message_title("Warning");

  win = new Fl_Double_Window(winw, winh, "CRC32 Check - drag and drop files");
  {
    int column_widths[] = { 82, 0 };
    browser = new Fl_Multi_Browser(10, 10, winw-20, winh-60);
    browser->column_widths(column_widths);
    browser->color(FL_WHITE);
    browser->callback(browser_cb);

    box = new dnd_box(10, 10, winw-20, winh-60);
    //box->tooltip("drag and drop files here");

    g = new Fl_Group(0, winh-40, winw, winh-50);
    {
      dummy = new Fl_Box(0, winh-40, winw-butw*3-30, 30);
      dummy->type(FL_NO_BOX);

      bt_copy = new Fl_Button(winw-butw*3-30, winh-40, butw, 30, "Copy CRC");
      bt_copy->deactivate();
      bt_copy->callback(copy_cb);

      bt_add = new Fl_Button(winw-butw*2-20, winh-40, butw, 30, "Add file");
      bt_add->callback(add_cb);

      bt_close = new Fl_Button(winw-butw-10, winh-40, butw, 30, "Close");
      bt_close->callback(close_cb);
    }
    g->resizable(dummy);
    g->end();
  }
  win->resizable(browser);
  win->position((Fl::w()-win->w())/2, (Fl::h()-win->h())/2);
  win->end();

  Fl::lock();
  win->show();

  pthread_t thread;
  pthread_create(&thread, 0, &get_crc_checksum, NULLPTR);

  return Fl::run();
}

