#include <stdio.h>
#include <cairo/cairo.h>
#include <gio/gio.h>
#include <librsvg/rsvg.h>

static cairo_status_t rsvg_cairo_write_func(void *closure, const unsigned char *data, unsigned int length)
{
  if (fwrite(data, 1, length, (FILE *)closure) == length)
  {
    return CAIRO_STATUS_SUCCESS;
  }
  return CAIRO_STATUS_WRITE_ERROR;
}

int svg_to_png_librsvg(const char *input, const char *output)
{
  RsvgHandle *rsvg = NULL;
  RsvgDimensionData dimensions;
  FILE *output_file = NULL;
  GError *error = NULL;
  GFile *file = NULL;
  GInputStream *stream = NULL;
  cairo_surface_t *surface = NULL;
  cairo_t *cr = NULL;

  file = g_file_new_for_path(input);
  stream = (GInputStream *)g_file_read(file, NULL, &error);
  if (stream == NULL)
  {
    g_printerr("%s: %s\n", input, error->message);
    g_error_free(error);
    return 1;
  }

  output_file = fopen(output, "wb");
  if (!output_file)
  {
    fprintf(stderr, "Error saving to file:\n%s\n", output);
    return 1;
  }

  rsvg = rsvg_handle_new_from_stream_sync(stream, file, RSVG_HANDLE_FLAGS_NONE, NULL, &error);
  if (error)
  {
    g_printerr("%s\n", error->message);
    g_error_free(error);
    return 1;
  }

  rsvg_handle_get_dimensions(rsvg, &dimensions);
  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, dimensions.width, dimensions.height);
  cr = cairo_create(surface);
  rsvg_handle_render_cairo(rsvg, cr);
  cairo_surface_write_to_png_stream(surface, rsvg_cairo_write_func, output_file);

  printf("Saved to file: %s\n", output);

  cairo_destroy(cr);
  cairo_surface_destroy(surface);
  fclose(output_file);
  rsvg_cleanup();

  return 0;
}

