/* GdkPixbuf library - Glycin image loader
 *
 * Copyright (C) 2024 Red Hat, Inc.
 *
 * Authors: Matthias Clasen <mclasenredhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <stdint.h>

#include <gio/gio.h>
#include <errno.h>
#include <glib/gi18n-lib.h>
#include "gdk-pixbuf-core.h"
#include "gdk-pixbuf-io.h"

#include <glycin.h>

static gboolean
should_run_unsandboxed (void)
{
  if (strcmp (g_get_prgname (), "gdk-pixbuf-thumbnailer") == 0)
    {
      g_debug ("In gdk-pixbuf-thumbnailer. Assuming external sandbox.");
      return TRUE;
    }

  return FALSE;
}

typedef struct _GlycinContext GlycinContext;
struct _GlycinContext
{
  GdkPixbufModulePreparedFunc prepared_func;
  GdkPixbufModuleUpdatedFunc updated_func;
  gpointer user_data;

  GFile *file;
  GIOStream *iostream;
  gboolean all_ok;
};

static gpointer
gdk_pixbuf__glycin_image_begin_load (GdkPixbufModuleSizeFunc       size_func,
                                     GdkPixbufModulePreparedFunc   prepared_func,
                                     GdkPixbufModuleUpdatedFunc    updated_func,
                                     gpointer                      user_data,
                                     GError                      **error)
{
  GlycinContext *context;
  GFile *file;
  GFileIOStream *iostream;

  g_assert (size_func != NULL);
  g_assert (prepared_func != NULL);
  g_assert (updated_func != NULL);

  context = g_new (GlycinContext, 1);
  context->prepared_func = prepared_func;
  context->updated_func = updated_func;
  context->user_data = user_data;
  context->all_ok = TRUE;
  file = g_file_new_tmp ("gdk-pixbuf-glycin-tmp.XXXXXX", &iostream, error);
  if (file == NULL)
    {
      g_free (context);
      return NULL;
    }

  context->file = file;
  context->iostream = G_IO_STREAM (iostream);

  return context;
}

static GdkPixbuf *
convert_glycin_frame_to_pixbuf (GlyFrame  *frame,
                                GError   **error)
{
  GdkPixbuf *pixbuf;
  GBytes *bytes;
  guchar *data;
  int R, G, B, A;
  int bpp;
  GlyMemoryFormat format;

  format = gly_frame_get_memory_format (frame);
  switch ((int) format)
    {
    case GLY_MEMORY_B8G8R8A8:
      B = 0; G = 1; R = 2; A = 3;
      bpp = 4;
      break;

    case GLY_MEMORY_A8R8G8B8:
      A = 0; R = 1; G = 2; B = 3;
      bpp = 4;
      break;

    case GLY_MEMORY_R8G8B8A8:
      R = 0; G = 1; B = 2; A = 3;
      bpp = 4;
      break;

    case GLY_MEMORY_A8B8G8R8:
      A = 0; B = 1; G = 2; R = 3;
      bpp = 4;
      break;

    case GLY_MEMORY_R8G8B8:
      R = 0; G = 1; B = 2; A = -1;
      bpp = 3;
      break;

    case GLY_MEMORY_B8G8R8:
      B = 0; G = 1; R = 2; A = -1;
      bpp = 3;
      break;

    default:
      g_set_error (error,
                   G_IO_ERROR,
                   G_IO_ERROR_FAILED,
                   "Glycin memory format %u not handled", format);
      return NULL;
    }

  bytes = gly_frame_get_buf_bytes (frame);
  data = g_bytes_get_data (bytes, NULL);
  pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8,
                           gly_frame_get_width (frame),
                           gly_frame_get_height (frame));

  for (gsize y = 0; y < gly_frame_get_height (frame); y++)
    {
      guchar *src = data + y * gly_frame_get_stride (frame);
      guchar *dst = gdk_pixbuf_get_pixels (pixbuf) + y * gdk_pixbuf_get_rowstride (pixbuf);

      for (gsize x = 0; x < gly_frame_get_width (frame); x++, src += bpp, dst += 4)
        {
          dst[0] = src[R];
          dst[1] = src[G];
          dst[2] = src[B];
          dst[3] = A != -1 ? src[A] : 0xff;
        }
    }

  return pixbuf;
}

static GdkPixbuf *
load_pixbuf_with_glycin (GFile   *file,
                         GError **error)
{
  GlyLoader *loader;
  GlyImage *image;
  GlyFrame *frame;
  GdkPixbuf *pixbuf = NULL;

  loader = gly_loader_new (file);

  if (should_run_unsandboxed ())
    gly_loader_set_sandbox_selector (loader, GLY_SANDBOX_SELECTOR_NOT_SANDBOXED);

  image = gly_loader_load (loader, error);
  if (!image)
    goto done;

  frame = gly_image_next_frame (image, error);
  if (!frame)
    goto done;

  pixbuf = convert_glycin_frame_to_pixbuf (frame, error);

done:
  g_clear_object (&loader);
  g_clear_object (&image);
  g_clear_object (&frame);

  return pixbuf;
}

static gboolean
gdk_pixbuf__glycin_image_stop_load (gpointer   data,
                                    GError   **error)
{
  GlycinContext *context = (GlycinContext *) data;
  gboolean retval = FALSE;

  g_return_val_if_fail (data != NULL, FALSE);

  if (!g_io_stream_close (context->iostream, NULL, error))
    context->all_ok = FALSE;

  if (context->all_ok)
    {
      GdkPixbuf *pixbuf = load_pixbuf_with_glycin (context->file, error);

      if (pixbuf != NULL)
        {
          (* context->prepared_func) (pixbuf,
                                      NULL,
                                      context->user_data);
          (* context->updated_func) (pixbuf,
                                     0, 0,
                                     gdk_pixbuf_get_width (pixbuf),
                                     gdk_pixbuf_get_height (pixbuf),
                                     context->user_data);
          g_object_unref (pixbuf);

          retval = TRUE;
        }
    }

  g_file_delete (context->file, NULL, NULL);

  g_clear_object (&context->file);
  g_clear_object (&context->iostream);
  g_free (context);

  return retval;
}

static gboolean
gdk_pixbuf__glycin_image_load_increment (gpointer       data,
                                         const guchar  *buf,
                                         guint          size,
                                         GError       **error)
{
  GlycinContext *context = (GlycinContext *) data;
  GOutputStream *ostream;
  gsize written;

  g_return_val_if_fail (data != NULL, FALSE);

  ostream = g_io_stream_get_output_stream (context->iostream);
  if (!g_output_stream_write_all (ostream, buf, size, &written, NULL, error) ||
      written != size)
    {
      context->all_ok = FALSE;
      return FALSE;
    }

  return TRUE;
}

void
glycin_fill_vtable (GdkPixbufModule *module)
{
  module->begin_load = gdk_pixbuf__glycin_image_begin_load;
  module->stop_load = gdk_pixbuf__glycin_image_stop_load;
  module->load_increment = gdk_pixbuf__glycin_image_load_increment;
}
