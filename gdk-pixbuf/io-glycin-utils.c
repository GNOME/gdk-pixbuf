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
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <gio/gio.h>
#include <errno.h>
#include "gdk-pixbuf-core.h"
#include "gdk-pixbuf-io.h"

#include <glycin.h>

static gboolean
should_run_unsandboxed (void)
{
  if (g_strcmp0 (g_get_prgname (), "gdk-pixbuf-thumbnailer") == 0)
    {
      g_debug ("In gdk-pixbuf-thumbnailer. Assuming external sandbox.");
      return TRUE;
    }

  if (g_strcmp0 (g_get_prgname (), "gdk-pixbuf-csource") == 0 ||
      g_strcmp0 (g_get_prgname (), "gdk-pixbuf-pixdata") == 0)
    {
      g_debug ("In %s. Not using sandboxing in build tools.", g_get_prgname ());
      return TRUE;
    }

  return FALSE;
}

typedef struct _GlycinContext GlycinContext;
struct _GlycinContext
{
  GdkPixbufModuleSizeFunc size_func;
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
  context->size_func = size_func;
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
convert_glycin_frame_to_pixbuf (GlyFrame *frame)
{
  GBytes *bytes;
  GlyMemoryFormat format;

  bytes = gly_frame_get_buf_bytes (frame);
  format = gly_frame_get_memory_format (frame);

  return gdk_pixbuf_new_from_bytes (bytes,
                                    GDK_COLORSPACE_RGB,
                                    gly_memory_format_has_alpha (format),
                                    8,
                                    gly_frame_get_width (frame),
                                    gly_frame_get_height (frame),
                                    gly_frame_get_stride (frame));
}

static GdkPixbuf *
load_pixbuf_with_glycin (GFile                    *file,
                         GdkPixbufModuleSizeFunc   size_func,
                         gpointer                  user_data,
                         GError                  **error)
{
  GlyLoader *loader;
  GlyImage *image;
  GlyFrameRequest *request = NULL;
  GlyFrame *frame = NULL;
  GdkPixbuf *pixbuf = NULL;
  GError *local_error = NULL;
  int width, height;
  char **keys;
  char value[64];

  loader = gly_loader_new (file);

  if (should_run_unsandboxed ())
    gly_loader_set_sandbox_selector (loader, GLY_SANDBOX_SELECTOR_NOT_SANDBOXED);

  gly_loader_set_accepted_memory_formats (loader, GLY_MEMORY_SELECTION_R8G8B8A8 |
                                                  GLY_MEMORY_SELECTION_R8G8B8);

  gly_loader_set_apply_transformations (loader, FALSE);

  image = gly_loader_load (loader, &local_error);
  if (!image)
    goto done;

  width = gly_image_get_width (image);
  height = gly_image_get_height (image);
  if (size_func)
    size_func (&width, &height, user_data);

  request = gly_frame_request_new ();

  gly_frame_request_set_scale (request, width, height);

  frame = gly_image_get_specific_frame (image, request, &local_error);
  if (!frame)
    goto done;

  pixbuf = convert_glycin_frame_to_pixbuf (frame);

  g_snprintf (value, sizeof (value), "%u",
              gly_image_get_transformation_orientation (image));
  gdk_pixbuf_set_option (pixbuf, "orientation", value);

  keys = gly_image_get_metadata_keys (image);
  if (keys)
    {
      for (gsize i = 0; keys[i]; i++)
        {
          char *value = gly_image_get_metadata_key_value (image, keys[i]);
          char *key = g_strconcat ("tEXt::", keys[i], NULL);
          gdk_pixbuf_set_option (pixbuf, key, value);
          g_free (key);
          g_free (value);
        }
      g_strfreev (keys);
    }

done:
  g_clear_object (&loader);
  g_clear_object (&image);
  g_clear_object (&request);
  g_clear_object (&frame);

  if (g_error_matches (local_error, GLY_LOADER_ERROR, GLY_LOADER_ERROR_FAILED))
    {
      g_set_error_literal (error, GDK_PIXBUF_ERROR, GDK_PIXBUF_ERROR_CORRUPT_IMAGE,
                           local_error->message);
      g_clear_error (&local_error);
    }
  else if (g_error_matches (local_error, GLY_LOADER_ERROR, GLY_LOADER_ERROR_UNKNOWN_IMAGE_FORMAT))
    {
      g_set_error_literal (error, GDK_PIXBUF_ERROR, GDK_PIXBUF_ERROR_UNKNOWN_TYPE,
                           local_error->message);
      g_clear_error (&local_error);
    }
  else if (local_error)
    {
      g_propagate_error (error, local_error);
    }

  return pixbuf;
}

#ifdef HAVE_READLINK
static GFile *
g_file_from_file (FILE    *f,
                  GError **error)
{
  char proc_path[256];
  char filename[256] = { 0, };
  ssize_t s;

  g_snprintf (proc_path, sizeof (proc_path), "/proc/%u/fd/%d", getpid (), fileno (f));
  s = readlink (proc_path, filename, sizeof (filename));

  if (s < 0)
    {
      g_set_error_literal (error,
                           G_IO_ERROR, G_IO_ERROR_FAILED,
                           "Failed to get the filename");
      return NULL;
    }
  else if (s == sizeof (filename))
    {
      g_set_error_literal (error,
                           G_IO_ERROR, G_IO_ERROR_FAILED,
                           "Filename too long");
      return NULL;
    }

  return g_file_new_for_path (filename);
}
#else
static GFile *
g_file_from_file (FILE    *f,
                  GError **error)
{
  g_set_error_literal (error,
                       G_IO_ERROR, G_IO_ERROR_FAILED,
                       "Failed to wrap FILE in GFile");
  return NULL;
}
#endif

static GdkPixbuf *
gdk_pixbuf__glycin_image_load (FILE *f, GError **error)
{
  GFile *file;
  GdkPixbuf *pixbuf;

  file = g_file_from_file (f, error);
  if (!file)
    return NULL;

  pixbuf = load_pixbuf_with_glycin (file, NULL, NULL, error);
  g_object_unref (file);

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
      GdkPixbuf *pixbuf = load_pixbuf_with_glycin (context->file,
                                                   context->size_func,
                                                   context->user_data,
                                                   error);

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

gboolean
glycin_image_save (const char         *mimetype,
                   FILE               *f,
                   GdkPixbufSaveFunc   save_func,
                   gpointer            user_data,
                   GdkPixbuf          *pixbuf,
                   char              **keys,
                   char              **values,
                   GBytes             *icc_profile,
                   int                 quality,
                   int                 compression,
                   GError            **error)
{
  GBytes *data;
  gsize length;
  guint width, height, stride;
  GlyMemoryFormat format;
  GlyCreator *creator;
  GlyNewFrame *frame;
  GlyEncodedImage *encoded_image;
  GBytes *binary_data;
  gboolean res;
  const char *image_data;

  creator = gly_creator_new (mimetype, error);
  if (!creator)
    return FALSE;

  if (should_run_unsandboxed ())
    gly_creator_set_sandbox_selector (creator, GLY_SANDBOX_SELECTOR_NOT_SANDBOXED);

  data = gdk_pixbuf_read_pixel_bytes (pixbuf);
  width = gdk_pixbuf_get_width (pixbuf);
  height = gdk_pixbuf_get_height (pixbuf);
  stride = gdk_pixbuf_get_rowstride (pixbuf);
  format = gdk_pixbuf_get_n_channels (pixbuf) == 3
             ? GLY_MEMORY_R8G8B8
             : GLY_MEMORY_R8G8B8A8;

  frame = gly_creator_add_frame_with_stride (creator,
                                             width, height,
                                             stride,
                                             format,
                                             data,
                                             error);
  g_bytes_unref (data);

  if (!frame)
    {
      g_object_unref (creator);
      return FALSE;
    }

  if (keys)
    {
      for (int i = 0; keys[i]; i++)
        gly_creator_add_metadata_key_value (creator, keys[i], values[i]);
    }

  if (icc_profile)
    gly_new_frame_set_color_icc_profile (frame, icc_profile);

  if (quality != -1)
    gly_creator_set_encoding_quality (creator, quality);

  if (compression != -1)
    gly_creator_set_encoding_compression (creator, compression);

  encoded_image = gly_creator_create (creator, error);

  g_object_unref (creator);
  g_object_unref (frame);

  if (!encoded_image)
    return FALSE;

  binary_data = gly_encoded_image_get_data (encoded_image);
  image_data = g_bytes_get_data (binary_data, &length);

  if (f)
    {
      GFile *file = g_file_from_file (f, error);

      if (!file)
        res = FALSE;
      else
        {
          res = g_file_replace_contents (file,
                                         image_data, length,
                                         NULL,
                                         FALSE,
                                         G_FILE_CREATE_NONE,
                                         NULL,
                                         NULL,
                                         error);
          g_object_unref (file);
        }
    }
  else
    {
      res = save_func (image_data, length, error, user_data);
    }

  g_bytes_unref (binary_data);
  g_object_unref (encoded_image);

  return res;
}

void
glycin_fill_vtable (GdkPixbufModule *module)
{
  module->load = gdk_pixbuf__glycin_image_load;
  module->begin_load = gdk_pixbuf__glycin_image_begin_load;
  module->stop_load = gdk_pixbuf__glycin_image_stop_load;
  module->load_increment = gdk_pixbuf__glycin_image_load_increment;
}
