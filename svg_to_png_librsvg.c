// gcc -Wall -O2 -DUSE_DLOPEN svg_to_png_librsvg.c -o svg_to_png_librsvg -s -ldl
// gcc -Wall -O2 $(pkg-config --cflags librsvg-2.0 cairo gio-2.0 glib-2.0) svg_to_png_librsvg.c -o svg_to_png_librsvg -s -lrsvg-2 -lcairo -lgio-2.0 -lglib-2.0 -lpthread

#include <stdio.h>

#ifdef USE_DLOPEN

#include <dlfcn.h>

// glib typedefs
typedef double gdouble;
typedef void* gpointer;
typedef char gchar;
typedef int gint;
typedef gint gboolean;
typedef unsigned int guint;
typedef unsigned int guint32;
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


static cairo_status_t rsvg_cairo_write_func(void *closure, const unsigned char *data, unsigned int length)
{
  if (fwrite(data, 1, length, (FILE *)closure) == length)
  {
    return 0;
  }
  return 1;
}

int svg_to_png_librsvg(const char *input, const char *output)
{
  INIT_DLOPEN

  LOAD_LIBRARY( h_rsvg,  "librsvg-2.so.2",   /**/ )
  LOAD_LIBRARY( h_cairo, "libcairo.so.2",    dlclose(h_rsvg) )
  LOAD_LIBRARY( h_gio,   "libgio-2.0.so.0",  dlclose(h_rsvg); dlclose(h_cairo) )
  LOAD_LIBRARY( h_glib,  "libglib-2.0.so.0", dlclose(h_rsvg); dlclose(h_cairo); dlclose(h_glib) )

  LOAD_SYMBOL( h_rsvg,  RsvgHandle*,       rsvg_handle_new_from_stream_sync,  (GInputStream *, GFile *, cairo_status_t, GCancellable *, GError **) )
  LOAD_SYMBOL( h_rsvg,  void,              rsvg_handle_get_dimensions,        (RsvgHandle *, RsvgDimensionData *) )
  LOAD_SYMBOL( h_rsvg,  gboolean,          rsvg_handle_render_cairo,          (RsvgHandle *, cairo_t *) )
  LOAD_SYMBOL( h_rsvg,  void,              rsvg_cleanup,                      (void) )
  LOAD_SYMBOL( h_cairo, cairo_t*,          cairo_create,                      (cairo_surface_t *) )
  LOAD_SYMBOL( h_cairo, void,              cairo_destroy,                     (cairo_t *) )
  LOAD_SYMBOL( h_cairo, cairo_surface_t*,  cairo_image_surface_create,        (cairo_status_t, int, int) )
  LOAD_SYMBOL( h_cairo, void,              cairo_surface_destroy,             (cairo_surface_t *) )
  LOAD_SYMBOL( h_cairo, cairo_status_t,    cairo_surface_write_to_png_stream, (cairo_surface_t *, cairo_write_func_t, void *) )
  LOAD_SYMBOL( h_gio,   GFile*,            g_file_new_for_path,               (const char *) )
  LOAD_SYMBOL( h_gio,   GFileInputStream*, g_file_read,                       (GFile *, GCancellable *, GError **) )
  LOAD_SYMBOL( h_glib,  void,              g_error_free,                      (GError *) )

  RsvgHandle *rsvg;
  RsvgDimensionData dimensions;
  FILE *output_file;
  GError *error;
  GFile *file;
  GInputStream *stream;
  cairo_surface_t *surface;
  cairo_t *cr;

  file = dl_g_file_new_for_path(input);
  stream = (GInputStream *)dl_g_file_read(file, NULL, &error);
  if (error)
  {
    fprintf(stderr, "%s: `%s'\n", error->message, input);
    dl_g_error_free(error);
    DLCLOSE_ALL
    return 1;
  }

  output_file = fopen(output, "wb");
  if (!output_file)
  {
    fprintf(stderr, "Error saving to file `%s'\n", output);
    DLCLOSE_ALL
    return 1;
  }

  rsvg = dl_rsvg_handle_new_from_stream_sync(stream, file, 0, NULL, &error);
  if (error)
  {
    fprintf(stderr, "%s\n", error->message);
    dl_g_error_free(error);
    fclose(output_file);
    DLCLOSE_ALL
    return 1;
  }

  dl_rsvg_handle_get_dimensions(rsvg, &dimensions);
  surface = dl_cairo_image_surface_create(0, dimensions.width, dimensions.height);
  cr = dl_cairo_create(surface);
  dl_rsvg_handle_render_cairo(rsvg, cr);

  if (dl_cairo_surface_write_to_png_stream(surface, rsvg_cairo_write_func, output_file) == 0)
  {
    printf("Saved to file `%s'\n", output);
  }
  else
  {
    fprintf(stderr, "Error saving to file `%s'\n", output);
  }

  dl_cairo_destroy(cr);
  dl_cairo_surface_destroy(surface);
  dl_rsvg_cleanup();
  fclose(output_file);
  DLCLOSE_ALL

  return 0;
}

int main(void)
{
  const char *in = "fltk/Applications-multimedia.svg";
  const char *out = "Applications-multimedia.png";
  return svg_to_png_librsvg(in, out);
}

