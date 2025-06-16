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

#include <android/imagedecoder.h>
#include <android/bitmap.h>
#include <android/data_space.h>

#include "io-android-utils.h"

typedef struct
{
  GdkPixbufModuleUpdatedFunc  updated_func;
  GdkPixbufModulePreparedFunc prepared_func;
  GdkPixbufModuleSizeFunc     size_func;
  gpointer                    user_data;

  GByteArray *buffer;
} GdkPixbufAndroidLoaderContext;

static gboolean
gdk_pixbuf__android_check_error (gint rc, GError **error)
{
  if (rc == ANDROID_IMAGE_DECODER_SUCCESS)
    return FALSE;
  g_set_error (error, G_IO_ERROR, G_IO_ERROR_FAILED,
               "Android Image Decoder Error: %s", AImageDecoder_resultToString (rc));
  return TRUE;
}

static GdkPixbuf *
gdk_pixbuf__android_image_decoder_to_pixbuf (AImageDecoder *decoder,
                                             const AImageDecoderHeaderInfo *header,
                                             gint32 width, gint32 height,
                                             GdkPixbufAndroidLoaderContext *context,
                                             GError **error)
{
  GdkPixbuf *pixbuf;
  guint len;
  guchar *data;
  gint rc;
  g_return_val_if_fail (width >= 0 && height >= 0, NULL);
  AImageDecoder_setAndroidBitmapFormat (decoder, ANDROID_BITMAP_FORMAT_RGBA_8888);
  AImageDecoder_setUnpremultipliedRequired (decoder, TRUE);
  pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, width, height);
  if (!pixbuf)
    {
      g_set_error_literal (error,
                           GDK_PIXBUF_ERROR,
                           GDK_PIXBUF_ERROR_INSUFFICIENT_MEMORY,
                           "Insufficient memory to allocate image");
      return NULL;
    }

  data = gdk_pixbuf_get_pixels_with_length (pixbuf, &len);
  g_assert (len == height * AImageDecoder_getMinimumStride (decoder));

  if (context)
    (* context->prepared_func) (pixbuf, NULL, context->user_data);

  rc = AImageDecoder_decodeImage(decoder, data, gdk_pixbuf_get_rowstride (pixbuf), len);
  if (gdk_pixbuf__android_check_error (rc, error))
    {
      g_free (data);
      return NULL;
    }

  if (context)
    (* context->updated_func) (pixbuf,
                               0, 0,
                               width, height,
                               context->user_data);

  return pixbuf;
}

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
  header = AImageDecoder_getHeaderInfo (decoder);
  width = AImageDecoderHeaderInfo_getWidth (header);
  height = AImageDecoderHeaderInfo_getHeight (header);
  ret = gdk_pixbuf__android_image_decoder_to_pixbuf (decoder, header, width, height, NULL, error);
  AImageDecoder_delete (decoder);
  return ret;
}

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
  GdkPixbuf *ret;
  GdkPixbufAndroidLoaderContext *context = (GdkPixbufAndroidLoaderContext *)data;

  rc = AImageDecoder_createFromBuffer (context->buffer->data, context->buffer->len, &decoder);
  if (gdk_pixbuf__android_check_error (rc, error))
    {
      g_byte_array_unref (context->buffer);
      g_free (context);
      return FALSE;
    }

  header = AImageDecoder_getHeaderInfo (decoder);
  width = AImageDecoderHeaderInfo_getWidth (header);
  height = AImageDecoderHeaderInfo_getHeight (header);
  set_width = width;
  set_height = height;
  (* context->size_func) (&set_width, &set_height, context->user_data);
  // Let someone else (ideally GL) deal with upscaling
  if (set_width <= width && set_height <= height)
    {
      width = set_width;
      height = set_height;
      rc = AImageDecoder_setTargetSize (decoder, width, height);
      if (gdk_pixbuf__android_check_error (rc, error))
        {
          AImageDecoder_delete (decoder);
          g_byte_array_unref (context->buffer);
          g_free (context);
          return FALSE;
        }
    }

  ret = gdk_pixbuf__android_image_decoder_to_pixbuf (decoder, header, width, height, context, error);
  AImageDecoder_delete (decoder);
  g_byte_array_unref (context->buffer);
  g_free (context);
  if (ret == NULL)
      return FALSE;
  g_object_unref (ret);
  return TRUE;
}

void
gdk_pixbuf__android_fill_vtable (GdkPixbufModule *module)
{
  module->load = gdk_pixbuf__android_image_load;
  module->begin_load = gdk_pixbuf__android_image_begin_load;
  module->stop_load = gdk_pixbuf__android_image_stop_load;
  module->load_increment = gdk_pixbuf__android_image_load_increment;
}

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
