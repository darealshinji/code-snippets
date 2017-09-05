#include <FL/Fl.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_Tiled_Image.H>
#include <FL/Fl_Double_Window.H>

#include "Fl_SVG_Image.cxx"

#include <stdio.h>


/* Applications-other.svg from https://commons.wikimedia.org/wiki/Tango_icons
 * https://jakearchibald.github.io/svgomg/
 */
const char *svg_data_name = "Applications-other.svg";
char *svg_data = (char *)
  "<svg xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' width='48' height='48'><defs><linearG"
  "radient id='c'><stop offset='0' stop-color='#fff'/><stop offset='1' stop-color='#fff' stop-opacity='0'/></linearGradient"
  "><linearGradient id='b'><stop offset='0' stop-color='#fcaf3e'/><stop offset='1' stop-color='#d37f03'/></linearGradient><"
  "linearGradient id='a'><stop offset='0' stop-opacity='.327'/><stop offset='1' stop-opacity='0'/></linearGradient><radialG"
  "radient gradientUnits='userSpaceOnUse' r='11.69' fy='72.568' fx='14.288' cy='68.873' cx='14.288' gradientTransform='matr"
  "ix(1.4 0 0 .513 4.365 4.84)' id='d' xlink:href='#a'/><radialGradient xlink:href='#b' id='e' cx='24.286' cy='36.721' fx='"
  "24.286' fy='36.721' r='20.411' gradientUnits='userSpaceOnUse' gradientTransform='translate(-17.677 -26.728) scale(1.728)"
  "'/><linearGradient xlink:href='#c' id='f' x1='26.503' y1='10.027' x2='28.786' y2='52.807' gradientUnits='userSpaceOnUse'"
  "/></defs><sodipodi:namedview pagecolor='#ffffff' bordercolor='#666666' borderopacity='1' showgrid='false'/><path transfo"
  "rm='translate(-4.54 -7.795) scale(1.186)' d='M44.286 38.714a19.93 9.837 0 1 1-39.857 0 19.93 9.837 0 1 1 39.85 0z' sodip"
  "odi:ry='9.837' sodipodi:rx='19.929' sodipodi:cy='38.714' sodipodi:cx='24.357' sodipodi:type='arc' color='#000' fill='url"
  "(#d)' fill-rule='evenodd' overflow='visible'/><path d='M24.286 43.196l-19.91-19.91 19.91-19.91 19.91 19.91-19.91 19.91z'"
  " fill='url(#e)' stroke='#f57900' stroke-linecap='round' stroke-linejoin='round'/><path d='M39.44 19.377L38.7 20.4c-.635-"
  ".268-1.3-.484-1.985-.644l.006-1.577c-.42-.09-.86-.16-1.3-.21l-.48 1.5c-.34-.03-.69-.05-1.04-.05-.35 0-.7.01-1.04.04l-.48"
  "-1.5c-.44.05-.87.11-1.3.2l.01 1.574c-.69.16-1.35.377-1.99.646l-.92-1.28c-.402.18-.794.38-1.175.6l.493 1.5c-.596.36-1.16."
  "77-1.686 1.226l-1.275-.934c-.324.3-.636.61-.932.937l.93 1.27c-.452.526-.864 1.09-1.226 1.69l-1.5-.495c-.22.38-.42.774-.6"
  " 1.176l1.28.92c-.27.637-.49 1.3-.65 1.986l-1.578-.003c-.09.425-.16.86-.207 1.3l1.505.48c-.03.345-.046.69-.046 1.045 0 .3"
  "5.018.7.047 1.04l-1.503.483c.048.44.116.874.204 1.3l1.578-.002c.16.687.377 1.35.645 1.987l-1.28.92c.18.403.38.795.6 1.17"
  "6l1.5-.493c.36.595.772 1.16 1.227 1.685l-.93 1.272c.28.307.57.6.876.88L43.3 23.23l-1.008-1.005-.3.218c-.525-.455-1.09-.8"
  "66-1.686-1.228l.242-.735-1.108-1.11v.002zM19.917 14.33c0 6.485-4.234 11.98-10.088 13.875l2.22 2.227.05-.02.18.252 1.41 1"
  ".412c.46-.22.91-.455 1.36-.708l-.7-2.11c.84-.51 1.63-1.088 2.37-1.728l1.79 1.312c.46-.418.896-.856 1.313-1.315l-1.31-1.7"
  "9c.64-.74 1.22-1.535 1.73-2.377l2.11.695c.305-.537.59-1.09.844-1.655l-1.8-1.298c.38-.897.68-1.83.905-2.797l2.22.01c.126-"
  ".604.22-1.217.29-1.837l-2.116-.68c.04-.483.063-.973.063-1.467 0-.49-.02-.98-.063-1.46l2.118-.68c-.07-.62-.17-1.23-.29-1."
  "83l-2.22.01c-.23-.96-.53-1.9-.91-2.79l1.8-1.3c-.14-.31-.295-.62-.45-.93l-3.8 3.806c.614 1.615.952 3.366.952 5.197z' opac"
  "ity='.5' fill='#fff'/><path d='M24.286 41.605l-18.32-18.32 18.32-18.32 18.32 18.32-18.32 18.32z' fill='none' stroke='url"
  "(#f)' opacity='.473'/></svg>";

#define TITLE_MAX 64


int main(int argc, char **argv)
{
  Fl_Double_Window *win;
  Fl_Group *g;
  Fl_SVG_Image *icon, *wp;
  char title[TITLE_MAX];
  int winw = 640;
  int winh = 480;

  icon = new Fl_SVG_Image("Applications-multimedia.svg");
  wp = new Fl_SVG_Image(72, 96, svg_data_name, svg_data);

  if (wp)
  {
    snprintf(title, TITLE_MAX - 1, "SVG test: %dx%d (%dx%d), %d%% zoom (X %d%%, Y %d%%)",
      wp->w(), wp->h(), wp->w_source(), wp->h_source(),
      (int)(wp->scale()*100.0), (int)(wp->scale_x()*100.0), (int)(wp->scale_y()*100.0));
  }
  else
  {
    snprintf(title, TITLE_MAX - 1, "SVG test: unknown resolution");
  }

  Fl_Window::default_icon(icon);

  win = new Fl_Double_Window(winw, winh, title);
  {
    g = new Fl_Group(0, 0, winw, winh);
    g->image(new Fl_Tiled_Image(wp));
    g->end();
  }
  win->position((Fl::w()-win->w())/2, (Fl::h()-win->h())/2);
  win->end();
  win->resizable(g);
  win->show(argc, argv);
  return Fl::run();
}

