/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017, djcj <djcj@gmx.de>
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

    dnd_box(int X, int Y, int W, int H, const char *L=0)
      : Fl_Box(X, Y, W, H, L) { }

    virtual ~dnd_box() { }

    int handle(int event);
};

Fl_Double_Window *win;
Fl_Multi_Browser *browser;
Fl_Button *bt_copy;
dnd_box *box;

#define MAX_ENTRIES 1000
std::string list[MAX_ENTRIES];
std::string list_bn[MAX_ENTRIES];
std::string list_crc[MAX_ENTRIES];
int current_line = 0;
int itemcount = 0;

void dnd_callback(const char *items);
void message_reached_max(void);
void split(const std::string &s, char c, std::vector<std::string> &v);
long calculate_crc32(char *file);

int dnd_box::handle(int event)
{
  int handle_ret = Fl_Box::handle(event);

  switch (event)
  {
    case FL_DND_ENTER:
    case FL_DND_DRAG:
    case FL_DND_RELEASE:
      handle_ret = 1;
      break;
    case FL_PASTE:
      {
        if (current_line >= MAX_ENTRIES)
        {
          message_reached_max();
        }
        else
        {
          dnd_callback(Fl::event_text());
        }
        handle_ret = 1;
        break;
      }
  }
  return handle_ret;
}

void split(const std::string &s, char c, std::vector<std::string> &v)
{
  size_t i = 0;
  size_t j = s.find(c);

  while (j != std::string::npos)
  {
    v.push_back(s.substr(i, j - i));
    i = ++j;
    j = s.find(c, j);

    if (j == std::string::npos)
    {
      v.push_back(s.substr(i, s.length()));
    }
  }
}

char from_hex(char ch)
{
  return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* http://www.geekhideout.com/urlcode.shtml */
char *url_decode(char *str)
{
  char *pstr = str;
  char *buf = new char[strlen(str) + 1];
  char *pbuf = buf;

  while (*pstr)
  {
    if (*pstr == '%')
    {
      if (pstr[1] && pstr[2])
      {
        *pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
        pstr += 2;
      }
    }
    else if (*pstr == '+')
    {
      *pbuf++ = ' ';
    }
    else
    {
      *pbuf++ = *pstr;
    }
    pstr++;
  }
  *pbuf = '\0';

  return buf;
}

void dnd_callback(const char *items)
{
  std::vector<std::string> vector;
  std::string entry;
  char *line;
  size_t i;

  if (itemcount < MAX_ENTRIES && strncmp(items, "file:///", 8) == 0 && strlen(items) > 8)
  {
    split(std::string(items), '\n', vector);

    for (i = 0; i < vector.size(); ++i)
    {
      line = url_decode((char *)vector[i].c_str() + 7);

      if (access(line, R_OK) == 0)
      {
        ++itemcount;

        if (itemcount < MAX_ENTRIES)
        {
          list[itemcount] = std::string(line);
          list_bn[itemcount] = std::string(basename(line));
          entry = "\t@." + list_bn[itemcount];
          browser->add(entry.c_str());
          win->redraw();
        }
      }
      delete line;
    }
  }
}

extern "C" void *get_crc_checksum(void *)
{
  long crc;
  std::string entry, color;
  std::stringstream ss;

  while (true)
  {
    if (current_line < itemcount)
    {
      ++current_line;

      if (current_line >= MAX_ENTRIES)
      {
        Fl::lock();
        message_reached_max();
        Fl::unlock();
        Fl::awake(win);
      }
      else
      {
        Fl::lock();
        browser->bottomline(current_line);
        Fl::unlock();
        Fl::awake(win);

        crc = calculate_crc32((char *)list[current_line].c_str());

        if (crc == -1)
        {
          entry = "@f@c@C88@.ERROR";  /* red */
        }
        else
        {
          ss << std::setfill('0') << std::setw(8) << std::hex << std::uppercase << crc;
          list_crc[current_line] = ss.str();
          ss.str(std::string());

          if (strcasestr(browser->text(current_line), list_crc[current_line].c_str()))
          {
            color = "60";  /* green */
          }
          else
          {
            color = "88";  /* red */
          }
          entry = "@f@c@C" + color + "@." + list_crc[current_line];
        }
        entry += "\t@." + list_bn[current_line];

        Fl::lock();
        browser->remove(current_line);
        browser->insert(current_line, entry.c_str());
        Fl::unlock();
        Fl::awake(win);
      }
    }

    usleep(1); /* needed, somehow... */
  }

  return NULLPTR;
}

long calculate_crc32(char *file)
{
  FILE *fp;
  uLong crc;
  size_t items;
  Bytef buf[262144]; /* 256k */
  long fileSize = 0L, byteCount = 0L;
  uInt completed = 0, n = 0;
  std::stringstream ss;
  std::string entry;

  fp = fopen(file, "r");

  if (fp == NULL)
  {
    return -1;
  }

  if (fseek(fp, 0, SEEK_END) == -1)
  {
    fclose(fp);
    return -1;
  }

  fileSize = ftell(fp);
  rewind(fp);
  crc = crc32(0, NULL, 0);

  while (feof(fp) == 0)
  {
    items = fread(buf, sizeof(*buf), sizeof(buf)/sizeof(*buf), fp);

    if (ferror(fp) != 0)
    {
      if (file != NULL)
      {
        fclose(fp);
      }
      return -1;
    }

    if (file != NULL)
    {
      byteCount += (long)(items * sizeof(*buf));
      n = (uInt)((float)byteCount/(float)fileSize*100.0);

      if (n > completed && n < 100)
      {
        completed = n;
        ss << completed;
        entry = "@f@c" + ss.str() + "%\t@." + list_bn[current_line];
        ss.str(std::string());

        Fl::lock();
        browser->remove(current_line);
        browser->insert(current_line, entry.c_str());
        Fl::unlock();
        Fl::awake(win);
      }
    }

    crc = crc32(crc, (const Bytef *)buf, (uInt)(items * sizeof(*buf)));
  }

  return (long)crc;
}

void message_reached_max(void)
{
  fl_message_title("Error");
  fl_alert("Maximum file entries reached!");
}

void browser_cb(Fl_Widget *)
{
  if (itemcount > 0)
  {
    bt_copy->activate();
  }
}

void add_cb(Fl_Widget *)
{
  Fl_Native_File_Chooser gtk;
  std::string entry;
  const char *file = NULL;
  const char *title = "Select a file";

  if (getenv("KDE_FULL_SESSION"))
  {
    /* don't use GTK file chooser on KDE, there may be layout issues */
    file = fl_file_chooser(title, "*", NULL);
  }
  else
  {
    gtk.title(title);
    gtk.type(Fl_Native_File_Chooser::BROWSE_FILE);

    if (gtk.show() == 0)
    {
      file = gtk.filename();
    }
  }

  if (!file || itemcount + 1 >= MAX_ENTRIES)
  {
    return;
  }

  ++itemcount;
  list[itemcount] = std::string(file);
  list_bn[itemcount] = std::string(basename(file));
  entry = "\t@." + list_bn[itemcount];
  browser->add(entry.c_str());
  win->redraw();
}

void copy_cb(Fl_Widget *)
{
  const char *text = list_crc[browser->value()].c_str();
  int len = strlen(text);

  if (len == 8)
  {
    Fl::copy(text, len, 1, Fl::clipboard_plain_text);
  }
}

void close_cb(Fl_Widget *)
{
  win->hide();
}

int main(void)
{
  Fl_Button *bt_open, *bt_close;
  Fl_Box *dummy;
  Fl_Group *g;
  pthread_t crc_thread;
  int winw = 640;
  int winh = 400;
  int butw = 90;
  int column_widths[] = { 82, 0 };

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

  win = new Fl_Double_Window(winw, winh, "CRC32 Check - drag and drop files");
  {
    browser = new Fl_Multi_Browser(10, 10, winw-20, winh-60);
    browser->column_widths(column_widths);
    browser->callback(browser_cb);

    box = new dnd_box(10, 10, winw-20, winh-60, NULL);
    //box->tooltip("drag and drop files here");

    g = new Fl_Group(0, winh-40, winw, winh-50);
    {
      dummy = new Fl_Box(0, winh-40, winw-butw*3-30, 30);
      dummy->type(FL_NO_BOX);

      bt_copy = new Fl_Button(winw-butw*3-30, winh-40, butw, 30, "Copy CRC");
      bt_copy->deactivate();
      bt_copy->callback(copy_cb);

      bt_open = new Fl_Button(winw-butw*2-20, winh-40, butw, 30, "Add file");
      bt_open->callback(add_cb);

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

  pthread_create(&crc_thread, 0, &get_crc_checksum, NULLPTR);

  return Fl::run();
}

