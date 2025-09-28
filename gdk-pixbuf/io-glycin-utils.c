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
#include <string.h>

#include <gio/gio.h>
#include <errno.h>
#include "gdk-pixbuf-core.h"
#include "gdk-pixbuf-io.h"
#include "gdk-pixbuf-animation.h"

#include <glycin.h>

/* {{{ Utilities */

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

/* }}} */
/* {{{ Animations */

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
static gint64
timeval_to_usec (const GTimeVal *timeval)
{
  return timeval->tv_sec * G_USEC_PER_SEC + timeval->tv_usec;
}

typedef struct {
  GdkPixbuf *pixbuf;
  gint64 delay; // usec
  gboolean is_last_frame;
} GdkPixbufGlycinFrame;

static void
gdk_pixbuf_glycin_frame_clear (GdkPixbufGlycinFrame *frame)
{
  g_object_unref (frame->pixbuf);
}

#define GDK_PIXBUF_TYPE_GLYCIN_ANIMATION (gdk_pixbuf_glycin_animation_get_type ())
#define GDK_PIXBUF_TYPE_GLYCIN_ANIMATION_ITER (gdk_pixbuf_glycin_animation_iter_get_type ())
G_DECLARE_FINAL_TYPE (GdkPixbufGlycinAnimation, gdk_pixbuf_glycin_animation, GDK_PIXBUF, GLYCIN_ANIMATION, GdkPixbufAnimation)
G_DECLARE_FINAL_TYPE (GdkPixbufGlycinAnimationIter, gdk_pixbuf_glycin_animation_iter, GDK_PIXBUF, GLYCIN_ANIMATION_ITER, GdkPixbufAnimationIter)

struct _GdkPixbufGlycinAnimation
{
  GdkPixbufAnimation parent_instance;
  gint32 width, height;

  GlyImage *image;
  GArray *decoded;
};

struct _GdkPixbufGlycinAnimationIter
{
  GdkPixbufAnimationIter parent_instance;

  GdkPixbufGlycinAnimation *animation;
  guint idx;
  gint64 time;
};

G_DEFINE_FINAL_TYPE (GdkPixbufGlycinAnimation, gdk_pixbuf_glycin_animation, GDK_TYPE_PIXBUF_ANIMATION)
G_DEFINE_FINAL_TYPE (GdkPixbufGlycinAnimationIter, gdk_pixbuf_glycin_animation_iter, GDK_TYPE_PIXBUF_ANIMATION_ITER)

static void
gdk_pixbuf_glycin_animation_finalize (GObject *object)
{
  GdkPixbufGlycinAnimation *self = (GdkPixbufGlycinAnimation *) object;

  g_array_unref (self->decoded);
  g_object_unref (self->image);

  G_OBJECT_CLASS (gdk_pixbuf_glycin_animation_parent_class)->finalize (object);
}

static gboolean
gdk_pixbuf_glycin_animation_is_static_image (G_GNUC_UNUSED GdkPixbufAnimation *animation)
{
  return FALSE;
}

static GdkPixbuf *
gdk_pixbuf_glycin_animation_get_static_image (GdkPixbufAnimation *animation)
{
  GdkPixbufGlycinAnimation *self = (GdkPixbufGlycinAnimation *) animation;

  if (self->decoded->len >= 1)
    return g_array_index (self->decoded, GdkPixbufGlycinFrame, 0).pixbuf;
  else
    return NULL;
}

static void
gdk_pixbuf_glycin_animation_get_size (GdkPixbufAnimation *animation,
                                      int                *width,
                                      int                *height)
{
  GdkPixbufGlycinAnimation *self = (GdkPixbufGlycinAnimation *) animation;

  if (width)
    *width = self->width;
  if (height)
    *height = self->height;
}

static GdkPixbufAnimationIter *
gdk_pixbuf_glycin_animation_get_iter (GdkPixbufAnimation *animation,
                                      const GTimeVal     *start_time)
{
  GdkPixbufGlycinAnimation *self = (GdkPixbufGlycinAnimation *) animation;
  GdkPixbufGlycinAnimationIter *iter;

  if (self->decoded->len > 0)
    {
      iter = g_object_new (GDK_PIXBUF_TYPE_GLYCIN_ANIMATION_ITER, NULL);
      iter->animation = g_object_ref (self);
      iter->idx = 0;
      iter->time = timeval_to_usec (start_time);
    }
  else
    iter = NULL;

  return (GdkPixbufAnimationIter *) iter;
}

static void
gdk_pixbuf_glycin_animation_class_init (GdkPixbufGlycinAnimationClass *klass)
{
  GObjectClass *object_class = (GObjectClass *) klass;
  GdkPixbufAnimationClass *animation_class = (GdkPixbufAnimationClass *) klass;

  object_class->finalize = gdk_pixbuf_glycin_animation_finalize;
  animation_class->is_static_image = gdk_pixbuf_glycin_animation_is_static_image;
  animation_class->get_static_image = gdk_pixbuf_glycin_animation_get_static_image;
  animation_class->get_size = gdk_pixbuf_glycin_animation_get_size;
  animation_class->get_iter = gdk_pixbuf_glycin_animation_get_iter;
}

static void
gdk_pixbuf_glycin_animation_init (GdkPixbufGlycinAnimation *self)
{
  self->width = -1;
  self->height = -1;
  self->image = NULL;
  self->decoded = g_array_new (FALSE, FALSE, sizeof (GdkPixbufGlycinFrame));
  g_array_set_clear_func (self->decoded,
                          (GDestroyNotify) gdk_pixbuf_glycin_frame_clear);
}

static void
gdk_pixbuf_glycin_animation_iter_finalize (GObject *object)
{
  GdkPixbufGlycinAnimationIter *self = (GdkPixbufGlycinAnimationIter *) object;

  g_object_unref (self->animation);

  G_OBJECT_CLASS (gdk_pixbuf_glycin_animation_iter_parent_class)->finalize (object);
}

static int
gdk_pixbuf_glycin_animation_iter_get_delay_time (GdkPixbufAnimationIter *iter)
{
  GdkPixbufGlycinAnimationIter *self = (GdkPixbufGlycinAnimationIter *) iter;
  int delay_time;

  delay_time = g_array_index (self->animation->decoded, GdkPixbufGlycinFrame, self->idx).delay / 1000; /* usec to millis */

  return delay_time;
}

static GdkPixbuf *
gdk_pixbuf_glycin_animation_iter_get_pixbuf (GdkPixbufAnimationIter *iter)
{
  GdkPixbufGlycinAnimationIter *self = (GdkPixbufGlycinAnimationIter *) iter;

  return g_array_index (self->animation->decoded, GdkPixbufGlycinFrame, self->idx).pixbuf;
}

static gboolean
gdk_pixbuf_glycin_animation_iter_on_currently_loading_frame (G_GNUC_UNUSED GdkPixbufAnimationIter *iter)
{
  return FALSE;
}

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
static gboolean
gdk_pixbuf_glycin_animation_iter_advance (GdkPixbufAnimationIter *iter,
                                          const GTimeVal         *current_time)
{
  GdkPixbufGlycinAnimationIter *self = (GdkPixbufGlycinAnimationIter *) iter;
  gint64 new_time;

  new_time = timeval_to_usec (current_time);

  while ((self->time + g_array_index (self->animation->decoded, GdkPixbufGlycinFrame, self->idx).delay) < new_time)
    {
      self->time += g_array_index (self->animation->decoded, GdkPixbufGlycinFrame, self->idx).delay;

      if (self->idx + 1 < self->animation->decoded->len)
        {
          self->idx++;
        }
      else if (g_array_index (self->animation->decoded, GdkPixbufGlycinFrame, self->idx).is_last_frame)
        {
          if (self->idx == 0)
            return FALSE;

          self->idx = 0;
        }
      else
        {
          GlyFrameRequest *request;
          GlyFrame *gly_frame;
          GError *error = NULL;

          request = gly_frame_request_new ();
          gly_frame_request_set_scale (request, self->animation->width, self->animation->height);
          gly_frame_request_set_loop_animation (request, FALSE);
          gly_frame = gly_image_get_specific_frame (self->animation->image,
                                                    request,
                                                    &error);
          g_object_unref (request);

          if (gly_frame)
            {
              GdkPixbufGlycinFrame frame;

              frame.pixbuf = convert_glycin_frame_to_pixbuf (gly_frame);
              frame.delay = gly_frame_get_delay (gly_frame);
              frame.is_last_frame = FALSE;
              g_array_append_val (self->animation->decoded, frame);
              g_object_unref (gly_frame);

              self->idx++;
            }
          else if (g_error_matches (error, GLY_LOADER_ERROR, GLY_LOADER_ERROR_NO_MORE_FRAMES))
            {
              g_array_index (self->animation->decoded, GdkPixbufGlycinFrame, self->idx).is_last_frame = TRUE;

              g_error_free (error);
              self->idx = 0;

              return FALSE;
            }
          else
            {
              g_error_free (error);
              return FALSE;
            }
        }
    }

  return TRUE;
}
G_GNUC_END_IGNORE_DEPRECATIONS

static void
gdk_pixbuf_glycin_animation_iter_class_init (GdkPixbufGlycinAnimationIterClass *klass)
{
  GObjectClass *object_class = (GObjectClass *) klass;
  GdkPixbufAnimationIterClass *iter_class = (GdkPixbufAnimationIterClass *) klass;

  object_class->finalize = gdk_pixbuf_glycin_animation_iter_finalize;

  iter_class->get_delay_time = gdk_pixbuf_glycin_animation_iter_get_delay_time;
  iter_class->get_pixbuf = gdk_pixbuf_glycin_animation_iter_get_pixbuf;
  iter_class->on_currently_loading_frame = gdk_pixbuf_glycin_animation_iter_on_currently_loading_frame;
  iter_class->advance = gdk_pixbuf_glycin_animation_iter_advance;
}

static void
gdk_pixbuf_glycin_animation_iter_init (GdkPixbufGlycinAnimationIter *self)
{
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
  GTimeVal current;

  g_get_current_time (&current);
  self->time = timeval_to_usec (&current);
G_GNUC_END_IGNORE_DEPRECATIONS

  self->animation = NULL;
  self->idx = 0;
}

static GdkPixbufGlycinAnimation *
gdk_pixbuf_glycin_animation_new (GlyImage *image,
                                 gint32    width,
                                 gint32    height)
{
  GdkPixbufGlycinAnimation *self;

  self = g_object_new (GDK_PIXBUF_TYPE_GLYCIN_ANIMATION, NULL);
  self->width = width;
  self->height = height;
  self->image = g_object_ref (image);

  return self;
}

G_GNUC_END_IGNORE_DEPRECATIONS

/* }}} */
/* {{{ Loading */

static GdkPixbuf *
load_pixbuf_with_glycin (GFile                    *file,
                         GdkPixbufModuleSizeFunc   size_func,
                         gpointer                  user_data,
                         GdkPixbufAnimation      **animation,
                         GError                  **error)
{
  GlyLoader *loader;
  GlyImage *image;
  GlyFrameRequest *request = NULL;
  GlyFrame *gly_frame = NULL;
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

  /* FIXME: Handle zero-size request and abort loading */
  if (width > 0 && height > 0)
    gly_frame_request_set_scale (request, width, height);

  gly_frame = gly_image_get_specific_frame (image, request, &local_error);
  if (!gly_frame)
    goto done;

  pixbuf = convert_glycin_frame_to_pixbuf (gly_frame);
  width = gdk_pixbuf_get_width (pixbuf);
  height = gdk_pixbuf_get_height (pixbuf);

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

  if (animation && gly_frame_get_delay (gly_frame) != 0)
    {
      GdkPixbufGlycinFrame frame;
      GdkPixbufGlycinAnimation *anim;

      anim = gdk_pixbuf_glycin_animation_new (image, width, height);
      frame.pixbuf = g_object_ref (pixbuf);
      frame.delay = gly_frame_get_delay (gly_frame);
      frame.is_last_frame = FALSE;
      g_array_append_val (anim->decoded, frame);

      *animation = (GdkPixbufAnimation *) anim;
    }
  else if (animation)
    *animation = NULL;

done:
  g_clear_object (&loader);
  g_clear_object (&image);
  g_clear_object (&request);
  g_clear_object (&gly_frame);

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

/* }}} */
/* {{{ Vfuncs */

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
gdk_pixbuf__glycin_image_load (FILE *f, GError **error)
{
  GFile *file;
  GdkPixbuf *pixbuf;

  file = g_file_from_file (f, error);
  if (!file)
    return NULL;

  pixbuf = load_pixbuf_with_glycin (file, NULL, NULL, NULL, error);
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
      GdkPixbuf *pixbuf;
      GdkPixbufAnimation *animation;

      pixbuf = load_pixbuf_with_glycin (context->file,
                                        context->size_func,
                                        context->user_data,
                                        &animation,
                                        error);
      if (pixbuf)
        {
          GBytes *bytes;
          const guchar *gly_data;
          gsize len;

          bytes = gdk_pixbuf_read_pixel_bytes (pixbuf);
          gly_data = g_bytes_get_data (bytes, &len);
          g_assert (gdk_pixbuf_read_pixels (pixbuf) == gly_data);

          (* context->prepared_func) (pixbuf, animation, context->user_data);

          if (gdk_pixbuf_read_pixels (pixbuf) != gly_data)
            {
              guchar *pixbuf_data;

              g_warning ("pixbuf was mutated in the prepare callback");

              pixbuf_data = gdk_pixbuf_get_pixels (pixbuf);
              memcpy (pixbuf_data, gly_data, len);
            }
          g_bytes_unref (bytes);

          (* context->updated_func) (pixbuf,
                                     0, 0,
                                     gdk_pixbuf_get_width (pixbuf),
                                     gdk_pixbuf_get_height (pixbuf),
                                     context->user_data);

          if (animation)
            g_object_unref (animation);
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

static GdkPixbufAnimation *
gdk_pixbuf__glycin_image_load_animation (FILE    *f,
                                         GError **error)
{
  GFile *file;
  GdkPixbufAnimation *animation = NULL;
  GdkPixbuf *pixbuf;

  file = g_file_from_file (f, error);
  if (!file)
    return NULL;

  pixbuf = load_pixbuf_with_glycin (file, NULL, NULL, &animation, error);
  g_object_unref (file);

  if (!pixbuf)
    return NULL;

  if (!animation)
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    animation = gdk_pixbuf_non_anim_new (pixbuf);
G_GNUC_END_IGNORE_DEPRECATIONS

  g_object_unref (pixbuf);

  return animation;
}

void
glycin_fill_vtable (GdkPixbufModule *module)
{
  module->load = gdk_pixbuf__glycin_image_load;
  module->begin_load = gdk_pixbuf__glycin_image_begin_load;
  module->stop_load = gdk_pixbuf__glycin_image_stop_load;
  module->load_increment = gdk_pixbuf__glycin_image_load_increment;
  module->load_animation = gdk_pixbuf__glycin_image_load_animation;
}

/* }}} */

/* vim:set foldmethod=marker: */
