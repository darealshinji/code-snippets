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
#include <FL/Fl_Multi_Browser.H>
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

#ifdef nullptr
#define NULLPTR nullptr
#else
void *null_ptr = NULL;
#define NULLPTR null_ptr
#endif

#define MAX_ENTRIES 1000

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
dnd_box *box;
std::string list[MAX_ENTRIES];
std::string list_crc[MAX_ENTRIES];
int current_line = 0;
int linecount = 0;
bool changed_tooltip = false;

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

  if (linecount < MAX_ENTRIES && strncmp(items, "file:///", 8) == 0 && strlen(items) > 8)
  {
    split(std::string(items), '\n', vector);

    for (i = 0; i < vector.size(); ++i)
    {
      line = url_decode((char *)vector[i].c_str() + 7);

      if (access(line, R_OK) == 0)
      {
        ++linecount;

        if (linecount < MAX_ENTRIES)
        {
          if (!changed_tooltip)
          {
            box->tooltip("select to copy checksum");
          }
          list[linecount] = std::string(line);
          entry = "\t@." + std::string(basename(list[linecount].c_str()));
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
  while (true)
  {
    if (current_line < linecount)
    {
      long crc;
      std::string entry = "@f@C88@.ERROR";  /* red */
      std::string color = "88";  /* red */
      std::stringstream ss;

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

        if (crc >= 0L)
        {
          ss << std::setfill('0') << std::setw(8) << std::hex << std::uppercase << crc;
          list_crc[current_line] = ss.str();

          if (strcasestr(browser->text(current_line), list_crc[current_line].c_str()))
          {
            color = "60";  /* green */
          }
          entry = "@f@C" + color + "@." + list_crc[current_line];
        }
        entry += std::string(browser->text(current_line));

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
  const char *text = list_crc[browser->value()].c_str();
  int len = strlen(text);

  if (len == 8)
  {
    Fl::copy(text, len, 1, Fl::clipboard_plain_text);
  }
}

int main(void)
{
  pthread_t crc_thread;
  int winw = 640;
  int winh = 400;
  int column_widths[] = { 82, 0 };

#ifdef WITH_ICON
  Fl_Pixmap pixmap(icon_xpm);
  Fl_RGB_Image icon(&pixmap, Fl_Color(0));
  Fl_Window::default_icon(&icon);
#endif
  Fl::visual(FL_DOUBLE|FL_INDEX);

  win = new Fl_Double_Window(winw, winh, "CRC32 Check  [drag & drop files]");
  {
    browser = new Fl_Multi_Browser(10, 10, winw-20, winh-20);
    browser->column_widths(column_widths);
    browser->callback(browser_cb);
    box = new dnd_box(0, 0, winw, winh, NULL);
    box->tooltip("drop files here");
  }
  win->resizable(browser);
  win->position((Fl::w() - win->w()) / 2, (Fl::h() - win->h()) / 2);
  win->end();

  Fl::lock();
  win->show();

  pthread_create(&crc_thread, 0, &get_crc_checksum, NULLPTR);

  return Fl::run();
}

