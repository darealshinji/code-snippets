// gcc -Wall -O2 -DUSE_DLOPEN svg_to_png_librsvg.c -o svg_to_png_librsvg -s -ldl
// gcc -Wall -O2 $(pkg-config --cflags librsvg-2.0 cairo gio-2.0 glib-2.0) svg_to_png_librsvg.c -o svg_to_png_librsvg -s -lrsvg-2 -lcairo -lgio-2.0 -lglib-2.0 -lpthread

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef USE_DLOPEN

#include <dlfcn.h>

// glib typedefs
typedef double gdouble;
typedef void* gpointer;
typedef char gchar;
typedef int gint;
typedef gint gboolean;
typedef unsigned int guint;
typedef unsigned char guint8;
typedef unsigned int guint32;
typedef unsigned long gsize;
typedef guint GType;
typedef guint32 GQuark;
typedef struct _GObject GObject;
typedef struct _GTypeInstance GTypeInstance;
typedef struct _GTypeClass GTypeClass;
typedef struct _GData GData;
typedef struct _GError GError;
struct _GTypeClass { GType g_type; };
struct _GTypeInstance { GTypeClass *g_class; };
struct _GObject { GTypeInstance g_type_instance; volatile guint ref_count; GData *qdata; };
struct _GError { GQuark domain; gint code; gchar *message; };

// gio typedefs
typedef struct _GFile GFile;
typedef struct _GInputStream GInputStream;
typedef struct _GInputStreamPrivate GInputStreamPrivate;
typedef struct _GFileInputStream GFileInputStream;
typedef struct _GFileInputStreamPrivate  GFileInputStreamPrivate;
typedef struct _GCancellable GCancellable;
typedef struct _GCancellablePrivate GCancellablePrivate;
struct _GInputStream { GObject parent_instance; GInputStreamPrivate *priv; };
struct _GFileInputStream { GInputStream parent_instance; GFileInputStreamPrivate *priv; };
struct _GCancellable { GObject parent_instance; GCancellablePrivate *priv; };

// cairo typedefs
typedef struct _cairo cairo_t;
typedef struct _cairo_surface cairo_surface_t;
typedef size_t cairo_status_t;  // actually an enum
typedef cairo_status_t (*cairo_write_func_t) (void *closure, const unsigned char *data, unsigned int length);

// rsvg typedefs
typedef struct _RsvgHandle RsvgHandle;
typedef struct  RsvgHandlePrivate RsvgHandlePrivate;
typedef struct _RsvgDimensionData RsvgDimensionData;
struct _RsvgHandle { GObject parent; RsvgHandlePrivate *priv; gpointer _abi_padding[15]; };
struct _RsvgDimensionData { int width; int height; gdouble em; gdouble ex; };

#define INIT_DLOPEN \
  const char *dlsym_error;

#define LOAD_LIBRARY(handle, library, ACTION_ON_ERROR) \
  void *handle = dlopen(library, RTLD_LAZY); \
  dlsym_error = dlerror(); \
  if (!handle) { \
    fprintf(stderr, "%s\n", dlsym_error); \
    ACTION_ON_ERROR; \
    return 1; \
  } \
  dlerror();

#define STRINGIFY(x)  #x

#define LOAD_SYMBOL(handle, type, symbol, param) \
  type (*dl_##symbol) param; \
  *(void **) (&dl_##symbol) = dlsym(handle, STRINGIFY(symbol)); \
  dlsym_error = dlerror(); \
  if (dlsym_error) { \
    fprintf(stderr, "Error: cannot load symbol\n%s\n", dlsym_error); \
    DLCLOSE_ALL; \
    return 1; \
  }

#define DLCLOSE_ALL \
  dlclose(h_rsvg); dlclose(h_cairo); dlclose(h_gio); dlclose(h_glib);

#else  // !USE_DLOPEN

#include <cairo/cairo.h>
#include <gio/gio.h>
#include <librsvg/rsvg.h>

#define INIT_DLOPEN
#define LOAD_LIBRARY(a,b,c)
#define LOAD_SYMBOL(a,b,c,d)
#define DLCLOSE_ALL
#define dl_rsvg_handle_new_from_stream_sync rsvg_handle_new_from_stream_sync
#define dl_rsvg_handle_new_from_data rsvg_handle_new_from_data
#define dl_rsvg_handle_get_dimensions rsvg_handle_get_dimensions
#define dl_rsvg_handle_render_cairo rsvg_handle_render_cairo
#define dl_rsvg_cleanup rsvg_cleanup
#define dl_cairo_create cairo_create
#define dl_cairo_destroy cairo_destroy
#define dl_cairo_image_surface_create cairo_image_surface_create
#define dl_cairo_surface_destroy cairo_surface_destroy
#define dl_cairo_surface_write_to_png_stream cairo_surface_write_to_png_stream
#define dl_g_file_new_for_path g_file_new_for_path
#define dl_g_file_read g_file_read
#define dl_g_error_free g_error_free

#endif  // USE_DLOPEN

char *svg_to_png_librsvg_buffer = NULL;
ssize_t svg_to_png_librsvg_buffer_length = 0;

#define svg_to_png_librsvg_copy(buffer, length) \
  buffer = (char *)malloc(svg_to_png_librsvg_buffer_length); \
  length = svg_to_png_librsvg_buffer_length; \
  memcpy(buffer, svg_to_png_librsvg_buffer, length);

#define svg_to_png_librsvg_free(x)       if (x) { free(x); }
#define svg_to_png_librsvg_from_file(x)  svg_to_png_librsvg(x, NULL)
#define svg_to_png_librsvg_from_data(x)  svg_to_png_librsvg(NULL, x)

void svg_to_png_librsvg_free_global(void)
{
  if (svg_to_png_librsvg_buffer)
  {
    free(svg_to_png_librsvg_buffer);
    svg_to_png_librsvg_buffer = NULL;
  }
  svg_to_png_librsvg_buffer_length = 0;
}

static inline
cairo_status_t rsvg_cairo_write_func(void *closure, const unsigned char *data, unsigned int length)
{
  int *pipefd = (int *)closure;
  if (write(pipefd[1], data, length) == length)
  {
    return 0;
  }
  return 1;
}

int svg_to_png_librsvg(const char *input_file, const char *input_data)
{
  INIT_DLOPEN

  LOAD_LIBRARY( h_rsvg,  "librsvg-2.so.2",   /**/ )
  LOAD_LIBRARY( h_cairo, "libcairo.so.2",    dlclose(h_rsvg) )
  LOAD_LIBRARY( h_gio,   "libgio-2.0.so.0",  dlclose(h_rsvg); dlclose(h_cairo) )
  LOAD_LIBRARY( h_glib,  "libglib-2.0.so.0", dlclose(h_rsvg); dlclose(h_cairo); dlclose(h_glib) )

  LOAD_SYMBOL( h_rsvg,  RsvgHandle *,       rsvg_handle_new_from_data,         (const guint8 *, gsize, GError **) )
  LOAD_SYMBOL( h_rsvg,  RsvgHandle *,       rsvg_handle_new_from_stream_sync,  (GInputStream *, GFile *, cairo_status_t, GCancellable *, GError **) )
  LOAD_SYMBOL( h_rsvg,  void,               rsvg_handle_get_dimensions,        (RsvgHandle *, RsvgDimensionData *) )
  LOAD_SYMBOL( h_rsvg,  gboolean,           rsvg_handle_render_cairo,          (RsvgHandle *, cairo_t *) )
  LOAD_SYMBOL( h_rsvg,  void,               rsvg_cleanup,                      (void) )
  LOAD_SYMBOL( h_cairo, cairo_t *,          cairo_create,                      (cairo_surface_t *) )
  LOAD_SYMBOL( h_cairo, void,               cairo_destroy,                     (cairo_t *) )
  LOAD_SYMBOL( h_cairo, cairo_surface_t *,  cairo_image_surface_create,        (cairo_status_t, int, int) )
  LOAD_SYMBOL( h_cairo, void,               cairo_surface_destroy,             (cairo_surface_t *) )
  LOAD_SYMBOL( h_cairo, cairo_status_t,     cairo_surface_write_to_png_stream, (cairo_surface_t *, cairo_write_func_t, void *) )
  LOAD_SYMBOL( h_gio,   GFile *,            g_file_new_for_path,               (const char *) )
  LOAD_SYMBOL( h_gio,   GFileInputStream *, g_file_read,                       (GFile *, GCancellable *, GError **) )
  LOAD_SYMBOL( h_glib,  void,               g_error_free,                      (GError *) )

  RsvgHandle *rsvg;
  RsvgDimensionData dimensions;
  GError *error;
  GFile *file;
  GInputStream *stream;
  cairo_surface_t *surface;
  cairo_t *cr;
  cairo_status_t save_status = 1;
  int pipefd[2];
  char buf;
  char *png_buf = NULL;
  ssize_t png_buf_length = 0;
  ssize_t length = 0;
  int rv = 1;

  if (input_data)
  {
    rsvg = dl_rsvg_handle_new_from_data((guint8 *)input_data, (gsize) strlen(input_data), &error);
  }
  else
  {
    file = dl_g_file_new_for_path(input_file);
    stream = (GInputStream *)dl_g_file_read(file, NULL, &error);
    if (error)
    {
      fprintf(stderr, "%s: `%s'\n", error->message, input_file);
      dl_g_error_free(error);
      DLCLOSE_ALL
      return 1;
    }
    rsvg = dl_rsvg_handle_new_from_stream_sync(stream, file, 0, NULL, &error);
  }

  if (error)
  {
    fprintf(stderr, "%s\n", error->message);
    dl_g_error_free(error);
    DLCLOSE_ALL
    return 1;
  }

  dl_rsvg_handle_get_dimensions(rsvg, &dimensions);
  surface = dl_cairo_image_surface_create(0, dimensions.width, dimensions.height);
  cr = dl_cairo_create(surface);
  dl_rsvg_handle_render_cairo(rsvg, cr);

  if (pipe(pipefd) == -1)
  {
    perror("pipe()");
    DLCLOSE_ALL
    return 1;
  }

  pid_t cpid = fork();
  if (cpid == -1)
  {
    perror("fork()");
    DLCLOSE_ALL
    return 1;
  }

  if (cpid == 0)
  {
    /* Child process */
    close(pipefd[0]);
    save_status = dl_cairo_surface_write_to_png_stream(surface, rsvg_cairo_write_func, pipefd);
    close(pipefd[1]);
    _exit(save_status);
  }
  else
  {
    close(pipefd[1]);
    while ((length = read(pipefd[0], &buf, 1)) > 0)
    {
      png_buf = (char *)realloc(png_buf, png_buf_length + length);
      memcpy(png_buf + png_buf_length, &buf, length);
      png_buf_length += length;
    }
    close(pipefd[0]);

    svg_to_png_librsvg_free_global();
    svg_to_png_librsvg_buffer = (char *)malloc(png_buf_length);
    svg_to_png_librsvg_buffer_length = png_buf_length;
    memcpy(svg_to_png_librsvg_buffer, png_buf, png_buf_length);
    free(png_buf);

    int wait_status;
    wait(&wait_status);
    if (wait_status == -1)
    {
      perror("wait()");
    }
    else
    {
      rv = 0;
    }
  }

  dl_cairo_destroy(cr);
  dl_cairo_surface_destroy(surface);
  dl_rsvg_cleanup();
  DLCLOSE_ALL

  return rv;
}

int main(void)
{
  int rv = 1;
  char *buf = NULL;
  ssize_t len = 0;

  const char *svg_data = /* Applications-other.svg */
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

  //rv = svg_to_png_librsvg_from_file("fltk/Applications-multimedia.svg");
  rv = svg_to_png_librsvg_from_data(svg_data);
  if (rv == 0)
  {
    svg_to_png_librsvg_copy(buf, len);
    fwrite(buf, len, 1, stdout);
    fflush(stdout);
  }
  svg_to_png_librsvg_free(buf);
  svg_to_png_librsvg_free_global();

  return rv;
}

