/* GdkPixbuf library - Android image loader
 *
 * Copyright (C) 2025  Florian Leander Singer <sp1rit@disroot.org>
 *
 * Authors: Florian Leander Singer <sp1rit@disroot.org>
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

#include <gio/gio.h>
#include <android/imagedecoder.h>
#include <android/bitmap.h>
#include <android/data_space.h>

#include "io-android-utils.h"

/* {{{ AImageDecoder utilities */

static gboolean
gdk_pixbuf__android_check_error (gint rc, GError **error)
{
  if (rc == ANDROID_IMAGE_DECODER_SUCCESS)
    return FALSE;
  g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
               "Android Image Decoder Error: %s", AImageDecoder_resultToString (rc));
  return TRUE;
}

static void
gdk_pixbuf__android_image_decoder_initial_setup (AImageDecoder *decoder)
{
  AImageDecoder_setAndroidBitmapFormat (decoder, ANDROID_BITMAP_FORMAT_RGBA_8888);
  AImageDecoder_setUnpremultipliedRequired (decoder, TRUE);
}

static GdkPixbuf *
gdk_pixbuf__android_allocate_pixbuf (gint32 width, gint32 height, GError **error)
{
  GdkPixbuf *pixbuf;
  g_return_val_if_fail (width >= 0 && height >= 0, NULL);

  pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, width, height);
  if (!pixbuf)
    {
      g_set_error_literal (error,
                           GDK_PIXBUF_ERROR,
                           GDK_PIXBUF_ERROR_INSUFFICIENT_MEMORY,
                           "Insufficient memory to allocate image");
      return NULL;
    }
  return pixbuf;
}

static gboolean
gdk_pixbuf__android_image_decoder_to_pixbuf (AImageDecoder *decoder,
                                             /*gint32 width, gint32 height,*/
                                             GdkPixbuf *pixbuf,
                                             GError **error)
{
  guint len;
  guchar *data;
  gint rc;

  data = gdk_pixbuf_get_pixels_with_length (pixbuf, &len);
  g_assert (len == gdk_pixbuf_get_height (pixbuf) * AImageDecoder_getMinimumStride (decoder));

  rc = AImageDecoder_decodeImage(decoder, data, gdk_pixbuf_get_rowstride (pixbuf), len);
  return !gdk_pixbuf__android_check_error (rc, error);
}

/* }}} */
/* {{{ Animations */

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
static gint64
gdk_pixbuf__android_anomation_iter_timeval_to_usec (const GTimeVal *timeval)
{
  return timeval->tv_sec * G_USEC_PER_SEC +
         timeval->tv_usec;
}
G_GNUC_END_IGNORE_DEPRECATIONS

typedef struct {
  GdkPixbuf *pixbuf;
  gint64 delay; // usec
} GdkPixbufAndroidFrame;

static void
gdk_pixbuf__android_frame_clear (GdkPixbufAndroidFrame *self)
{
  g_object_unref (self->pixbuf);
}

// ugly hack to work arround the fact that gdk-pixbuf doesn't expose autoptr
#undef _GLIB_DEFINE_AUTOPTR_CHAINUP
#define _GLIB_DEFINE_AUTOPTR_CHAINUP(ModuleObjName, ParentName)

#define GDK_PIXBUF_TYPE_ANDROID_ANIMATION (gdk_pixbuf__android_animation_get_type ())
#define GDK_PIXBUF_TYPE_ANDROID_ANIMATION_ITER (gdk_pixbuf__android_animation_iter_get_type ())
G_DECLARE_FINAL_TYPE (GdkPixbufAndroidAnimation, gdk_pixbuf__android_animation, GDK_PIXBUF, ANDROID_ANIMATION, GdkPixbufAnimation)
G_DECLARE_FINAL_TYPE (GdkPixbufAndroidAnimationIter, gdk_pixbuf__android_animation_iter, GDK_PIXBUF, ANDROID_ANIMATION_ITER, GdkPixbufAnimationIter)

struct _GdkPixbufAndroidAnimation
{
  GdkPixbufAnimation parent_instance;
  GBytes *imgdata;
  gint32 width,height;

  GMutex lock;
  AImageDecoder *decoder;
  GArray *decoded;
};

struct _GdkPixbufAndroidAnimationIter {
  GdkPixbufAnimationIter parent_instance;

  GdkPixbufAndroidAnimation *animation;
  guint idx;
  gint32 repeat_count;
  gint64 timestate;
  gboolean on_last_frame;
};

G_DEFINE_FINAL_TYPE (GdkPixbufAndroidAnimation, gdk_pixbuf__android_animation, GDK_TYPE_PIXBUF_ANIMATION)
G_DEFINE_FINAL_TYPE (GdkPixbufAndroidAnimationIter, gdk_pixbuf__android_animation_iter, GDK_TYPE_PIXBUF_ANIMATION_ITER)

static void
gdk_pixbuf__android_animation_finalize (GObject *object)
{
  GdkPixbufAndroidAnimation *self = (GdkPixbufAndroidAnimation *)object;
  AImageDecoder_delete (self->decoder);
  if (self->imgdata)
    g_bytes_unref (self->imgdata);
  g_array_unref (self->decoded);
  g_mutex_clear (&self->lock);
  G_OBJECT_CLASS (gdk_pixbuf__android_animation_parent_class)->finalize (object);
}

static gboolean
gdk_pixbuf__android_animation_is_static_image (G_GNUC_UNUSED GdkPixbufAnimation *animation)
{
  // An animation is never a static image
  return FALSE;
}

static GdkPixbuf *
gdk_pixbuf__android_animation_get_static_image (GdkPixbufAnimation *animation)
{
  GdkPixbuf *ret;
  GdkPixbufAndroidAnimation *self = (GdkPixbufAndroidAnimation *)animation;

  g_mutex_lock (&self->lock);
  if (self->decoded->len >= 1)
    ret = g_array_index (self->decoded, GdkPixbufAndroidFrame, 0).pixbuf;
  else
    ret = NULL;
  g_mutex_unlock (&self->lock);
  return ret;
}

static void
gdk_pixbuf__android_animation_get_size (GdkPixbufAnimation *animation, int *width, int *height)
{
  GdkPixbufAndroidAnimation *self = (GdkPixbufAndroidAnimation *)animation;
  if (width)
    *width = self->width;
  if (height)
    *height = self->height;
}

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
static GdkPixbufAnimationIter *
gdk_pixbuf__android_animation_get_iter (GdkPixbufAnimation* animation, const GTimeVal* start_time)
{
  GdkPixbufAndroidAnimationIter *iter;
  GdkPixbufAndroidAnimation *self = (GdkPixbufAndroidAnimation *)animation;
  g_mutex_lock (&self->lock);
  if (self->decoded->len > 0)
    {
      iter = g_object_new (GDK_PIXBUF_TYPE_ANDROID_ANIMATION_ITER, NULL);
      iter->animation = g_object_ref (self);
      iter->idx = 0;
      iter->repeat_count = AImageDecoder_getRepeatCount (self->decoder);
      iter->timestate = gdk_pixbuf__android_anomation_iter_timeval_to_usec (start_time);
      iter->on_last_frame = FALSE;
    }
  else
    iter = NULL;
  g_mutex_unlock (&self->lock);
  return (GdkPixbufAnimationIter *)iter;
}
G_GNUC_END_IGNORE_DEPRECATIONS

static void
gdk_pixbuf__android_animation_class_init (GdkPixbufAndroidAnimationClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;
  GdkPixbufAnimationClass *animation_class = (GdkPixbufAnimationClass *)klass;

  object_class->finalize = gdk_pixbuf__android_animation_finalize;
  animation_class->is_static_image = gdk_pixbuf__android_animation_is_static_image;
  animation_class->get_static_image = gdk_pixbuf__android_animation_get_static_image;
  animation_class->get_size = gdk_pixbuf__android_animation_get_size;
  animation_class->get_iter = gdk_pixbuf__android_animation_get_iter;
}

static void
gdk_pixbuf__android_animation_init (GdkPixbufAndroidAnimation *self)
{
  self->imgdata = NULL;
  self->width = -1;
  self->height = -1;
  g_mutex_init (&self->lock);
  self->decoder = NULL;
  self->decoded = g_array_new (FALSE, FALSE, sizeof (GdkPixbufAndroidFrame));
  g_array_set_clear_func (self->decoded, (GDestroyNotify)gdk_pixbuf__android_frame_clear);
}

static void
gdk_pixbuf__android_animation_iter_finalize (GObject *object)
{
  GdkPixbufAndroidAnimationIter *self = (GdkPixbufAndroidAnimationIter *)object;
  g_object_unref (self->animation);
  G_OBJECT_CLASS (gdk_pixbuf__android_animation_iter_parent_class)->finalize (object);
}

static gint
gdk_pixbuf__android_animation_iter_get_delay_time (GdkPixbufAnimationIter *iter)
{
  GdkPixbufAndroidAnimationIter *self;
  gint delay_time;
  self = (GdkPixbufAndroidAnimationIter *)iter;
  if (self->on_last_frame)
    return -1;
  g_mutex_lock (&self->animation->lock);
  delay_time = g_array_index (self->animation->decoded, GdkPixbufAndroidFrame, self->idx).delay / 1000 /* usec to millis */;
  g_mutex_unlock (&self->animation->lock);
  return delay_time;
}

static GdkPixbuf *
gdk_pixbuf__android_animation_iter_get_pixbuf (GdkPixbufAnimationIter *iter)
{
  GdkPixbufAndroidAnimationIter *self;
  GdkPixbuf *ret;
  self = (GdkPixbufAndroidAnimationIter *)iter;
  g_mutex_lock (&self->animation->lock);
  ret = g_array_index (self->animation->decoded, GdkPixbufAndroidFrame, self->idx).pixbuf;
  g_mutex_unlock (&self->animation->lock);
  return ret;
}

static gboolean
gdk_pixbuf__android_animation_iter_on_currently_loading_frame (G_GNUC_UNUSED GdkPixbufAnimationIter *iter)
{
  return FALSE;
}

static gint64
gdk_pixbuf__android_get_delay_for_current_frame (AImageDecoder *decoder)
{
  gint64 duration;
  AImageDecoderFrameInfo *frameInfo = AImageDecoderFrameInfo_create();
  AImageDecoder_getFrameInfo (decoder, frameInfo);
  duration = AImageDecoderFrameInfo_getDuration(frameInfo);
  AImageDecoderFrameInfo_delete(frameInfo);
  if (duration < 0)
    return duration;
  return duration / 1000 /* nanos to usec */;
}

G_GNUC_BEGIN_IGNORE_DEPRECATIONS
static gboolean
gdk_pixbuf__android_animation_iter_advance (GdkPixbufAnimationIter *iter, const GTimeVal *current_time)
{
  GdkPixbufAndroidAnimationIter *self;
  gint64 new_time;
  gboolean retval;
  self = (GdkPixbufAndroidAnimationIter *)iter;
  if (self->on_last_frame)
    return FALSE;

  new_time = gdk_pixbuf__android_anomation_iter_timeval_to_usec (current_time);
  g_mutex_lock (&self->animation->lock);
  retval = FALSE;
  while ((self->timestate + g_array_index (self->animation->decoded, GdkPixbufAndroidFrame, self->idx).delay) < new_time)
    {
      if (self->animation->decoded->len <= self->idx+1) // currently on last decoded frame
        {
          gint rc;
          GError *error;
          GdkPixbuf *pixbuf;
          GdkPixbufAndroidFrame frame;
          rc = AImageDecoder_advanceFrame (self->animation->decoder);
          if (rc == ANDROID_IMAGE_DECODER_FINISHED)
            {
              if (self->repeat_count == ANDROID_IMAGE_DECODER_INFINITE)
                {
                  self->timestate += g_array_index (self->animation->decoded, GdkPixbufAndroidFrame, self->idx).delay;
                  self->idx = 0;
                  retval = TRUE;
                }
              else
                {
                  if (self->repeat_count > 0)
                    {
                      self->timestate += g_array_index (self->animation->decoded, GdkPixbufAndroidFrame, self->idx).delay;
                      self->idx = 0;
                      retval = TRUE;
                      --self->repeat_count;
                    }
                  else
                    self->on_last_frame = TRUE;
                }
              break;
            }
          else if (rc != ANDROID_IMAGE_DECODER_SUCCESS)
            {
              g_critical ("Android animation decoder failure: %s", AImageDecoder_resultToString(rc));
              break;
            }
          error = NULL;
          pixbuf = gdk_pixbuf__android_allocate_pixbuf (self->animation->width, self->animation->height, &error);
          if (!pixbuf)
            {
              g_critical ("Failed to allocate android pixbuf: %s", error->message);
              g_error_free (error);
              break;
            }
          if (!gdk_pixbuf__android_image_decoder_to_pixbuf (self->animation->decoder, pixbuf, &error))
            {
              g_critical ("Android animation decoder failure: %s", error->message);
              g_error_free (error);
              break;
            }

          frame.pixbuf = pixbuf;
          frame.delay = gdk_pixbuf__android_get_delay_for_current_frame (self->animation->decoder);
          g_array_append_val (self->animation->decoded, frame);
        }
      self->timestate += g_array_index (self->animation->decoded, GdkPixbufAndroidFrame, self->idx).delay;
      self->idx++;
      retval = TRUE;
    }
  g_mutex_unlock (&self->animation->lock);
  return retval;
}
G_GNUC_END_IGNORE_DEPRECATIONS

static void
gdk_pixbuf__android_animation_iter_class_init (GdkPixbufAndroidAnimationIterClass *klass)
{
  GObjectClass *object_class = (GObjectClass *)klass;
  GdkPixbufAnimationIterClass *iter_class = (GdkPixbufAnimationIterClass *)klass;

  object_class->finalize = gdk_pixbuf__android_animation_iter_finalize;
  iter_class->get_delay_time = gdk_pixbuf__android_animation_iter_get_delay_time;
  iter_class->get_pixbuf = gdk_pixbuf__android_animation_iter_get_pixbuf;
  iter_class->on_currently_loading_frame = gdk_pixbuf__android_animation_iter_on_currently_loading_frame;
  iter_class->advance = gdk_pixbuf__android_animation_iter_advance;
}

static void
gdk_pixbuf__android_animation_iter_init (GdkPixbufAndroidAnimationIter *self)
{
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
  GTimeVal current;
  g_get_current_time (&current);
  self->timestate = gdk_pixbuf__android_anomation_iter_timeval_to_usec (&current);
G_GNUC_END_IGNORE_DEPRECATIONS

  self->animation = NULL;
  self->idx = 0;
  self->repeat_count = -1;
  self->on_last_frame = FALSE;
}

static GdkPixbufAndroidAnimation *
gdk_pixbuf__android_get_animation (GBytes *imgdata /* takes ownership */,
                                   AImageDecoder *decoder /* takes ownership */,
                                   gint32 width, gint32 height)
{
  GdkPixbufAndroidAnimation *self;
  g_assert (AImageDecoder_isAnimated (decoder)); // may only be called for animated images

  self = g_object_new (GDK_PIXBUF_TYPE_ANDROID_ANIMATION, NULL);
  self->imgdata = imgdata;
  self->width = width;
  self->height = height;
  self->decoder = decoder;

  return self;
}

/* }}} */
/* {{{ Module implementation */

static GdkPixbuf *
gdk_pixbuf__android_image_load (FILE *f, GError **error)
{
  AImageDecoder *decoder;
  gint rc;
  const AImageDecoderHeaderInfo *header;
  gint32 width,height;
  GdkPixbuf *ret;

  rc = AImageDecoder_createFromFd (fileno (f), &decoder);
  if (gdk_pixbuf__android_check_error (rc, error))
    return NULL;
  gdk_pixbuf__android_image_decoder_initial_setup (decoder);
  header = AImageDecoder_getHeaderInfo (decoder);
  width = AImageDecoderHeaderInfo_getWidth (header);
  height = AImageDecoderHeaderInfo_getHeight (header);
  ret = gdk_pixbuf__android_allocate_pixbuf (width, height, error);
  if (!ret)
    {
      AImageDecoder_delete (decoder);
      return NULL;
    }
  if (!gdk_pixbuf__android_image_decoder_to_pixbuf (decoder, ret, error))
    {
      g_object_unref (ret);
      AImageDecoder_delete (decoder);
      return NULL;
    }
  AImageDecoder_delete (decoder);
  return ret;
}

typedef struct
{
  GdkPixbufModuleSizeFunc     size_func;
  GdkPixbufModulePreparedFunc prepared_func;
  GdkPixbufModuleUpdatedFunc  updated_func;
  gpointer                    user_data;

  GByteArray *buffer;
} GdkPixbufAndroidLoaderContext;

static gpointer
gdk_pixbuf__android_image_begin_load (GdkPixbufModuleSizeFunc       size_func,
                                      GdkPixbufModulePreparedFunc   prepared_func,
                                      GdkPixbufModuleUpdatedFunc    updated_func,
                                      gpointer                      user_data,
                                      GError                      **error)
{
  GdkPixbufAndroidLoaderContext *context = g_new (GdkPixbufAndroidLoaderContext, 1);
  context->buffer = g_byte_array_new ();
  context->size_func     = size_func;
  context->prepared_func = prepared_func;
  context->updated_func  = updated_func;
  context->user_data     = user_data;
  return context;
}

static gboolean
gdk_pixbuf__android_image_load_increment (gpointer       data,
                                          const guchar  *buf,
                                          guint          size,
                                          GError       **error)
{
  GdkPixbufAndroidLoaderContext *context = (GdkPixbufAndroidLoaderContext *)data;
  g_byte_array_append (context->buffer, buf, size);
  return TRUE;
}

static gboolean
gdk_pixbuf__android_image_stop_load (gpointer data, GError **error)
{
  AImageDecoder *decoder;
  gint rc;
  const AImageDecoderHeaderInfo *header;
  gint32 width,height,set_width,set_height;
  GdkPixbuf *pixbuf;
  GdkPixbufAndroidLoaderContext *context = (GdkPixbufAndroidLoaderContext *)data;
  GBytes *imgdata = g_byte_array_free_to_bytes (context->buffer);

  rc = AImageDecoder_createFromBuffer (context->buffer->data, context->buffer->len, &decoder);
  if (gdk_pixbuf__android_check_error (rc, error))
    {
      g_bytes_unref (imgdata);
      g_free (context);
      return FALSE;
    }

  gdk_pixbuf__android_image_decoder_initial_setup (decoder);
  header = AImageDecoder_getHeaderInfo (decoder);
  width = AImageDecoderHeaderInfo_getWidth (header);
  height = AImageDecoderHeaderInfo_getHeight (header);
  set_width = width;
  set_height = height;
  (* context->size_func) (&set_width, &set_height, context->user_data);
  // Let someone else (ideally GL) deal with upscaling
  if (set_width <= width && set_height <= height && (width != set_width || height != set_height))
    {
      width = set_width;
      height = set_height;
      rc = AImageDecoder_setTargetSize (decoder, width, height);
      if (gdk_pixbuf__android_check_error (rc, error))
        {
          AImageDecoder_delete (decoder);
          g_bytes_unref (imgdata);
          g_free (context);
          return FALSE;
        }
    }

  pixbuf = gdk_pixbuf__android_allocate_pixbuf (width, height, error);
  if (!pixbuf)
    {
      AImageDecoder_delete (decoder);
      g_bytes_unref (imgdata);
      g_free (context);
      return FALSE;
    }

  if (AImageDecoder_isAnimated (decoder))
    {
      GdkPixbufAndroidAnimation *animation;
      GdkPixbufAndroidFrame frame;
      animation = gdk_pixbuf__android_get_animation (imgdata, decoder, width, height);
      g_mutex_lock (&animation->lock);
      frame.pixbuf = pixbuf;
      frame.delay = gdk_pixbuf__android_get_delay_for_current_frame (animation->decoder);
      g_array_append_val (animation->decoded, frame);
      g_mutex_unlock (&animation->lock);

      (* context->prepared_func) (pixbuf, (GdkPixbufAnimation *)animation, context->user_data);
      if (!gdk_pixbuf__android_image_decoder_to_pixbuf (animation->decoder, pixbuf, error))
        {
          g_object_unref (animation);
          g_free (context);
          return FALSE;
        }
      (* context->updated_func) (pixbuf, 0, 0, width, height, context->user_data);

      g_object_unref (animation);
      g_free (context);
      return TRUE;
    }
  else
    {
      (* context->prepared_func) (pixbuf, NULL, context->user_data);
      if (!gdk_pixbuf__android_image_decoder_to_pixbuf (decoder, pixbuf, error))
        {
          AImageDecoder_delete (decoder);
          g_bytes_unref (imgdata);
          g_object_unref (pixbuf);
          g_free (context);
          return FALSE;
        }
      (* context->updated_func) (pixbuf, 0, 0, width, height, context->user_data);
      AImageDecoder_delete (decoder);
      g_bytes_unref (imgdata);
      g_object_unref (pixbuf);
      g_free (context);
      return TRUE;
    }
}

static GdkPixbufAnimation *
gdk_pixbuf__android_image_load_animation (FILE *f, GError **error)
{
  AImageDecoder *decoder;
  gint rc;
  const AImageDecoderHeaderInfo *header;
  gint32 width,height;
  GdkPixbuf *initial_frame;
  GdkPixbufAnimation *animation;
  GdkPixbufAndroidFrame frame;

  rc = AImageDecoder_createFromFd (fileno (f), &decoder);
  if (gdk_pixbuf__android_check_error (rc, error))
    return NULL;
  gdk_pixbuf__android_image_decoder_initial_setup (decoder);
  header = AImageDecoder_getHeaderInfo (decoder);
  width = AImageDecoderHeaderInfo_getWidth (header);
  height = AImageDecoderHeaderInfo_getHeight (header);

  initial_frame = gdk_pixbuf__android_allocate_pixbuf (width, height, error);
  if (!initial_frame)
    {
      AImageDecoder_delete (decoder);
      return FALSE;
    }

  if (!gdk_pixbuf__android_image_decoder_to_pixbuf (decoder, initial_frame, error))
    {
      AImageDecoder_delete (decoder);
      g_object_unref (initial_frame);
      return FALSE;
    }

  if (!AImageDecoder_isAnimated (decoder))
    {
      animation = gdk_pixbuf_non_anim_new (initial_frame);
      AImageDecoder_delete (decoder);
      g_object_unref (initial_frame);
      return animation;
    }

  animation = (GdkPixbufAnimation *)gdk_pixbuf__android_get_animation (NULL, decoder,
                                                                       width, height);
  frame.pixbuf = initial_frame;
  frame.delay = gdk_pixbuf__android_get_delay_for_current_frame (((GdkPixbufAndroidAnimation *)animation)->decoder);
  g_array_append_val (((GdkPixbufAndroidAnimation *)animation)->decoded, frame);
  return animation;
}

void
gdk_pixbuf__android_fill_vtable (GdkPixbufModule *module)
{
  module->load = gdk_pixbuf__android_image_load;
  module->begin_load = gdk_pixbuf__android_image_begin_load;
  module->stop_load = gdk_pixbuf__android_image_stop_load;
  module->load_increment = gdk_pixbuf__android_image_load_increment;
  module->load_animation = gdk_pixbuf__android_image_load_animation;
}

/* }}} */
/* {{{ Save image functionality */

typedef struct
{
  GdkPixbufSaveFunc save_func;
  gpointer user_data;

  GError **error;
} GdkPixbufAndroidSaveImageData;

static bool
gdk_pixbuf__android_save_image_cb (gpointer user_data, const void *data, gsize size)
{
  GdkPixbufAndroidSaveImageData *save_image_data = (GdkPixbufAndroidSaveImageData *)user_data;
  return save_image_data->save_func (data, size, save_image_data->error, save_image_data->user_data);
}

gboolean
gdk_pixbuf__android_save_image (GdkPixbufSaveFunc save_func, gpointer user_data, GdkPixbuf *pixbuf, gint32 format, GError **error)
{
  AndroidBitmapInfo info;
  GdkPixbufAndroidSaveImageData data;
  gint rc;
  info = (AndroidBitmapInfo){
    .width = gdk_pixbuf_get_width (pixbuf),
    .height = gdk_pixbuf_get_height (pixbuf),
    .stride = gdk_pixbuf_get_rowstride (pixbuf),
    .format = ANDROID_BITMAP_FORMAT_RGBA_8888,
    .flags = ANDROID_BITMAP_FLAGS_ALPHA_UNPREMUL
  };
  data = (GdkPixbufAndroidSaveImageData){
    .save_func = save_func,
    .user_data = user_data,
    .error = error
  };
  rc = AndroidBitmap_compress(&info, ADATASPACE_SRGB,
                              gdk_pixbuf_get_pixels (pixbuf),
                              format, 90,
                              &data, gdk_pixbuf__android_save_image_cb);
  if (data.error && *data.error)
    return FALSE;
  if (rc == ANDROID_BITMAP_RESULT_SUCCESS)
    return TRUE;
  switch (rc)
    {
      case ANDROID_BITMAP_RESULT_BAD_PARAMETER:
        g_set_error_literal (error, G_IO_ERROR, GDK_PIXBUF_ERROR_FAILED, "Bad parameter");
        break;
      case ANDROID_BITMAP_RESULT_JNI_EXCEPTION:
        g_set_error_literal (error, G_IO_ERROR, GDK_PIXBUF_ERROR_FAILED, "JNI exception occured");
        break;
      case ANDROID_BITMAP_RESULT_ALLOCATION_FAILED:
        g_set_error_literal (error, G_IO_ERROR, GDK_PIXBUF_ERROR_INSUFFICIENT_MEMORY, "Allocation failed");
        break;
      default:
        g_set_error_literal (error, G_IO_ERROR, GDK_PIXBUF_ERROR_FAILED, "Unknown error");
    }
  return FALSE;
}

/* }}} */
/* vim:set foldmethod=marker: */
