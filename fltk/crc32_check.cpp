/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017-2019, djcj <djcj@gmx.de>
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

#include <iostream>
#include <string>
#include <vector>
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

#define XSTR(x) #x
#define STR(x)  XSTR(x)


class dnd_box : public Fl_Box
{
public:
  dnd_box(int X, int Y, int W, int H) : Fl_Box(X, Y, W, H) {}
  virtual ~dnd_box() {}
  int handle(int event);
};

Fl_Double_Window *win;
Fl_Multi_Browser *browser;
Fl_Button *bt_copy, *bt_add, *bt_clear;
dnd_box *box;

pthread_t t1;
std::vector<std::string> list, list_bn, list_crc;
int current_line = 0, itemcount = 0;
char crc_clipboard[10] = {0};

std::string bg[2] = {
  "@B255", /* white */
  "@B17"   /* light yellow */
};

void dnd_callback(const char *items);

int dnd_box::handle(int event)
{
  int ret = Fl_Box::handle(event);
  switch (event) {
    case FL_DND_ENTER:
    case FL_DND_DRAG:
    case FL_DND_RELEASE:
      ret = 1;
      break;
    case FL_PASTE:
      dnd_callback(Fl::event_text());
      ret = 1;
      break;
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
  if (strncmp(items, "file:///", 8) == 0 && strlen(items) > 8) {
    std::vector<std::string> vec;
    split(std::string(items), '\n', vec);

    for (size_t i = 0; i < vec.size(); i++) {
      char *line = strdup(vec[i].c_str() + 7);
      if (line && line[0] != '\0') {
        fl_decode_uri(line);
      }
      if (line && line[0] != '\0') {
        itemcount++;

        std::string s(line);
        list.push_back(s);
        char *bn = basename(line);
        list_bn.push_back(bn ? std::string(bn) : s);
        int j = itemcount % 2;
        std::string entry = bg[j] + "\t" + bg[j] + "@." + list_bn.back();

        browser->add(entry.c_str());
        bt_clear->activate();
        win->redraw();
        free(line);
      }
    }
  }
}

long calculate_crc32(const char *file)
{
  FILE *fp;
  Bytef buf[524288]; /* 512k */
  long items;
  double fileSize, byteCount = 0;
  uInt completed = 0;
  uLong crc;

  if (!(fp = fopen(file, "r"))) {
    return -1;
  }

  if (fseek(fp, 0, SEEK_END) == -1) {
    fclose(fp);
    return -1;
  }

  fileSize = ftell(fp);
  rewind(fp);

  crc = crc32(0, NULL, 0);

  while (feof(fp) == 0) {
    items = fread(buf, sizeof(*buf), sizeof(buf)/sizeof(*buf), fp);

    if (ferror(fp) != 0) {
      fclose(fp);
      return -1;
    }

    byteCount += items * sizeof(*buf);
    uInt n = byteCount / fileSize * 100;

    if (n > completed && n < 100) {
      completed = n;
      int i = current_line % 2;
      char tmp[4];
      snprintf(tmp, 3, "%d", completed);
      std::string entry = bg[i] + "@f@c" + tmp + "%\t" + bg[i] + "@." + list_bn[current_line];

      Fl::lock();
      browser->remove(current_line);
      browser->insert(current_line, entry.c_str());
      Fl::unlock();
      Fl::awake(win);
    }

    crc = crc32(crc, buf, items * sizeof(*buf));
  }

  return crc;
}

void get_crc_checksum_real(void)
{
  if (current_line < itemcount) {
    current_line++;
    int i = current_line % 2;
    std::string entry = bg[i] + "@f@c0%\t" + bg[i] + "@." + list_bn[current_line];

    Fl::lock();
    browser->bottomline(current_line);
    browser->remove(current_line);
    browser->insert(current_line, entry.c_str());
    Fl::unlock();
    Fl::awake(win);

    entry = bg[i] + "@f@c";
    long crc = calculate_crc32(list[current_line].c_str());

    if (crc == -1) {
      entry += "@C88@.ERROR";  /* red */
      list_crc.push_back("ERROR");  /* don't leave list_crc entry empty */
    } else {
      char tmp[10];
      snprintf(tmp, 9, "%08lX", crc);
      list_crc.push_back(tmp);

      if (strcasestr(list_bn[current_line].c_str(), list_crc.back().c_str())) {
        entry += "@C60";  /* green */
      } else {
        entry += "@C88";  /* red */
      }
      entry += "@." + list_crc.back();
    }
    entry += "\t" + bg[i] + "@." + list_bn[current_line];

    Fl::lock();
    browser->remove(current_line);
    browser->insert(current_line, entry.c_str());
    Fl::unlock();
    Fl::awake(win);
  }
}

extern "C" void *get_crc_checksum(void *)
  /* don't optimize this loop away */
  __attribute__ ((optimize("-O0")));

extern "C" void *get_crc_checksum(void *)
{
  while (true) {
    get_crc_checksum_real();
    usleep(10000); /* 10ms */
  }
  return NULL;
}

static void browser_cb(Fl_Widget *) {
  if (itemcount > 0 && !bt_copy->active()) {
    bt_copy->activate();
  }
}

/* GTK file chooser and multi-threading doesn't work so well;
 * disabling it might be a good idea */
#define WITH_GTK

static void add_cb(Fl_Widget *)
{
  const char *file = NULL, *title = "Select a file";

#ifndef WITH_GTK
  file = fl_file_chooser(title, "*", NULL);
#else
  Fl_Native_File_Chooser *gtk = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);
  gtk->title(title);
  if (gtk->show() == 0) {
    file = gtk->filename();
  }
#endif

  if (!file) {
    return;
  }

  itemcount++;
  list.push_back(std::string(file));

  char *copy = strdup(file);
  char *base = basename(copy);
  list_bn.push_back(base ? std::string(base) : std::string(copy));
  free(copy);

  int i = itemcount % 2;
  std::string entry = bg[i] + "\t" + bg[i] + "@." + list_bn.back();
  browser->add(entry.c_str());
  bt_clear->activate();
}

static void copy_cb(Fl_Widget *)
{
  const char *text = "";
  if (browser->value() <= current_line) {
    text = strncpy(crc_clipboard, list_crc[browser->value()].c_str(), 8);
  }
  Fl::copy(text, strlen(text), 1, Fl::clipboard_plain_text);
}

static void clear_cb(Fl_Widget *)
{
  pthread_cancel(t1);

  list.clear();
  list_bn.clear();
  list_crc.clear();
  list.push_back("");
  list_bn.push_back("");
  list_crc.push_back("");
  current_line = itemcount = 0;

  browser->clear();
  bt_copy->deactivate();
  bt_clear->deactivate();
  win->redraw();
  pthread_create(&t1, 0, &get_crc_checksum, NULL);
}

static void close_cb(Fl_Widget *) {
  pthread_cancel(t1);
  win->hide();
}

int main(void)
{
  Fl_Button *bt_close;
  Fl_Box *dummy, *info;
  Fl_Group *g;
  const int w = 640, h = 400, butw = 90;

  /* add an empty first entry, so that n in list[n]
   * equals browser->value() and current_line */
  list.push_back("");
  list_bn.push_back("");
  list_crc.push_back("");

#ifdef WITH_ICON
  Fl_Window::default_icon(new Fl_RGB_Image(new Fl_Pixmap(icon_xpm), Fl_Color(0)));
#endif
  Fl::scheme("gtk+");
  Fl::visual(FL_DOUBLE|FL_INDEX);

  std::string version_info =
    "FLTK " STR(FL_MAJOR_VERSION) "." STR(FL_MINOR_VERSION) "." STR(FL_PATCH_VERSION) " - http://fltk.org\n"
    "zlib " + std::string(zlibVersion()) + " - https://zlib.net";

  win = new Fl_Double_Window(w, h, "CRC32 Check - drag and drop files");
  {
    const int column_widths[] = { 82, 0 };
    browser = new Fl_Multi_Browser(10, 10, w - 20, h - 60);
    browser->column_widths(column_widths);
    browser->color(FL_WHITE);
    browser->callback(browser_cb);

    box = new dnd_box(10, 10, w - 20, h - 60);

    g = new Fl_Group(0, h - 40, w, h - 50);
    {
      bt_close = new Fl_Button(w - butw - 10, h - 40, butw, 30, "Close");
      bt_close->callback(close_cb);

      bt_add = new Fl_Button(bt_close->x() - butw - 10, h - 40, butw, 30, "Add file");
      bt_add->callback(add_cb);

      bt_clear = new Fl_Button(bt_add->x() - butw - 10, h - 40, butw, 30, "Clear");
      bt_clear->deactivate();
      bt_clear->callback(clear_cb);

      bt_copy = new Fl_Button(bt_clear->x() - butw - 10, h - 40, butw, 30, "Copy CRC");
      bt_copy->deactivate();
      bt_copy->callback(copy_cb);

      info = new Fl_Box(bt_copy->x() - 16, h - 40, 1, 30, version_info.c_str());
      info->type(FL_NO_BOX);
      info->align(FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);
      info->labelsize(11);
      info->deactivate();  /* gray font */

      dummy = new Fl_Box(0, h - 40, 1, 30);
      dummy->type(FL_NO_BOX);
    }
    g->resizable(dummy);
    g->end();
  }
  win->resizable(browser);
  win->position((Fl::w() - w)/2, (Fl::h() - h)/2);
  win->callback(close_cb);
  win->end();

  Fl::lock();
  win->show();

  pthread_create(&t1, 0, &get_crc_checksum, NULL);

  return Fl::run();
}

