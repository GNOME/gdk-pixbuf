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

#include "io-glycin-utils.h"


static gboolean
filter_keys (char    **keys,
             char    **values,
             GBytes  **icc_bytes,
             int      *quality,
             GError  **error)
{
  guchar *icc_data = NULL;
  gsize icc_length = 0;

  if (!keys)
    return TRUE;

  for (int i = 0; keys[i]; i++)
    {
      if (strcmp (keys[i], "icc-profile") == 0)
        {
          icc_data = g_base64_decode (values[i], &icc_length);
          if (icc_length < 127)
            {
              g_set_error (error,
                           GDK_PIXBUF_ERROR,
                           GDK_PIXBUF_ERROR_BAD_OPTION,
                           "Color profile has invalid length %lu",
                           icc_length);
              g_free (icc_data);
              return FALSE;
            }
        }
      else if (strcmp (keys[i], "quality") == 0)
        {
          char *endp = NULL;

          *quality = strtol (values[i], &endp, 10);
          if ((endp && *endp != '\0') || *quality < 0 || *quality > 100)
            {
              g_set_error (error,
                           GDK_PIXBUF_ERROR,
                           GDK_PIXBUF_ERROR_BAD_OPTION,
                           "JPEG quality must be a value between 0 and 100; “%s” could not be parsed.", values[i]);
              g_free (icc_data);
              return FALSE;
            }
        }
      else
        {
          g_set_error (error,
                       GDK_PIXBUF_ERROR,
                       GDK_PIXBUF_ERROR_BAD_OPTION,
                       "Unhandled key while saving: %s", keys[i]);
          g_free (icc_data);
          return FALSE;
        }
    }

  if (icc_data)
    *icc_bytes = g_bytes_new_take (icc_data, icc_length);

  return TRUE;
}

static gboolean
gdk_pixbuf__jpeg_image_save (FILE       *f,
                             GdkPixbuf  *pixbuf,
                             char      **keys,
                             char      **values,
                             GError    **error)
{
  GBytes *icc_data = NULL;
  int quality = -1;
  gboolean ret;

  if (!filter_keys (keys, values, &icc_data, &quality, error))
    return FALSE;

  ret = glycin_image_save ("image/jpeg", f, NULL, NULL,
                           pixbuf,
                           NULL, NULL,
                           icc_data,
                           quality, -1,
                           error);

  g_clear_pointer (&icc_data, g_bytes_unref);

  return ret;
}

static gboolean
gdk_pixbuf__jpeg_image_save_to_callback (GdkPixbufSaveFunc   save_func,
                                         gpointer            user_data,
                                         GdkPixbuf          *pixbuf,
                                         gchar             **keys,
                                         gchar             **values,
                                         GError            **error)
{
  GBytes *icc_data = NULL;
  int quality = -1;
  gboolean ret;

  if (!filter_keys (keys, values, &icc_data, &quality, error))
    return FALSE;

  ret = glycin_image_save ("image/jpeg", NULL, save_func, user_data,
                           pixbuf,
                           NULL, NULL,
                           icc_data,
                           quality, -1,
                           error);

  g_clear_pointer (&icc_data, g_bytes_unref);

  return ret;
}

static gboolean
gdk_pixbuf__jpeg_is_save_option_supported (const gchar *option_key)
{
  if (option_key == NULL)
    return FALSE;

  return strcmp (option_key, "icc-profile") == 0 ||
         strcmp (option_key, "quality") == 0;
}

#ifndef INCLUDE_glycin
#define MODULE_ENTRY(function) G_MODULE_EXPORT void function
#else
#define MODULE_ENTRY(function) void _gdk_pixbuf__glycin_jpeg_ ## function
#endif

MODULE_ENTRY (fill_vtable) (GdkPixbufModule *module)
{
  glycin_fill_vtable (module);

  module->save = gdk_pixbuf__jpeg_image_save;
  module->save_to_callback = gdk_pixbuf__jpeg_image_save_to_callback;
  module->is_save_option_supported = gdk_pixbuf__jpeg_is_save_option_supported;
}

MODULE_ENTRY (fill_info) (GdkPixbufFormat *info)
{
  static const GdkPixbufModulePattern signature[] = {
    { "\xff\xd8", NULL, 100 },
    { NULL, NULL, 0 }
  };
  static const gchar *mime_types[] = {
    "image/jpeg",
    NULL
  };
  static const gchar *extensions[] = {
    "jpeg",
    "jpe",
    "jpg",
    NULL
  };

  info->name = "jpeg";
  info->signature = (GdkPixbufModulePattern *) signature;
  info->description = "JPEG";
  info->mime_types = (gchar **) mime_types;
  info->extensions = (gchar **) extensions;
  info->flags = GDK_PIXBUF_FORMAT_WRITABLE | GDK_PIXBUF_FORMAT_THREADSAFE;
  info->license = "LGPL";
}
