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
             char   ***out_keys,
             char   ***out_values,
             GBytes  **icc_bytes,
             int      *compression,
             GError  **error)
{
  char **filtered_keys = NULL;
  char **filtered_values = NULL;
  guchar *icc_data = NULL;
  gsize icc_length = 0;
  guint length;

  if (!keys)
    return TRUE;

  length = g_strv_length (keys);

  filtered_keys = g_new0 (char *, length + 1);
  filtered_values = g_new0 (char *, length + 1);

  for (int i = 0, j = 0; i < length; i++)
    {
      if (g_str_has_prefix (keys[i], "tEXt::"))
        {
          filtered_keys[j] = keys[i] + strlen ("tEXt::");
          filtered_values[j] = values[i];
          j++;
        }
      else if (strcmp (keys[i], "icc-profile") == 0)
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
              g_free (filtered_keys);
              g_free (filtered_values);
              return FALSE;
            }
        }
      else if (strcmp (keys[i], "compression") == 0)
        {
          char *endp = NULL;

          *compression = strtol (values[i], &endp, 10);
          if ((endp && *endp != '\0') || *compression < 0 || *compression > 9)
            {
              g_set_error (error,
                           GDK_PIXBUF_ERROR,
                           GDK_PIXBUF_ERROR_BAD_OPTION,
                           "PNG compression level must be a value between 0 and 9; value “%s” is invalid",
                           values[i]);
              g_free (icc_data);
              g_free (filtered_keys);
              g_free (filtered_values);
              return FALSE;
            }

          *compression *= 11;
        }
      else
        {
          g_warning ("Unrecognized parameter “%s” passed to the PNG saver", keys[i]);
        }
    }

  if (filtered_keys[0])
    {
      *out_keys = filtered_keys;
      *out_values = filtered_values;
    }
  else
    {
      g_free (filtered_keys);
      g_free (filtered_values);
    }

  if (icc_data)
    *icc_bytes = g_bytes_new_take (icc_data, icc_length);

  return TRUE;
}

static gboolean
gdk_pixbuf__png_image_save (FILE       *f,
                            GdkPixbuf  *pixbuf,
                            char      **keys,
                            char      **values,
                            GError    **error)
{
  char **filtered_keys = NULL;
  char **filtered_values = NULL;
  GBytes *icc_data = NULL;
  int compression = -1;
  gboolean ret;

  if (!filter_keys (keys, values,
                    &filtered_keys, &filtered_values,
                    &icc_data,
                    &compression,
                    error))
    return FALSE;

  ret = glycin_image_save ("image/png", f, NULL, NULL,
                           pixbuf,
                           filtered_keys, filtered_values,
                           icc_data,
                           -1, compression,
                           error);

  g_clear_pointer (&icc_data, g_bytes_unref);
  g_free (filtered_keys);
  g_free (filtered_values);

  return ret;
}

static gboolean
gdk_pixbuf__png_image_save_to_callback (GdkPixbufSaveFunc   save_func,
                                        gpointer            user_data,
                                        GdkPixbuf          *pixbuf,
                                        gchar             **keys,
                                        gchar             **values,
                                        GError            **error)
{
  char **filtered_keys = NULL;
  char **filtered_values = NULL;
  GBytes *icc_data = NULL;
  int compression = -1;
  gboolean ret;

  if (!filter_keys (keys, values,
                    &filtered_keys, &filtered_values,
                    &icc_data,
                    &compression,
                    error))
    return FALSE;

  ret = glycin_image_save ("image/png", NULL, save_func, user_data,
                           pixbuf,
                           filtered_keys, filtered_values,
                           icc_data,
                           -1, compression,
                           error);

  g_clear_pointer (&icc_data, g_bytes_unref);
  g_free (filtered_keys);
  g_free (filtered_values);

  return ret;
}

static gboolean
gdk_pixbuf__png_is_save_option_supported (const gchar *option_key)
{
  if (option_key == NULL)
    return FALSE;

  return strcmp (option_key, "icc-profile") == 0 ||
         strcmp (option_key, "compression") == 0 ||
         strncmp (option_key, "tEXt::", strlen ("tEXt::")) == 0;
}

#ifndef INCLUDE_glycin
#define MODULE_ENTRY(function) G_MODULE_EXPORT void function
#else
#define MODULE_ENTRY(function) void _gdk_pixbuf__glycin_png_ ## function
#endif

MODULE_ENTRY (fill_vtable) (GdkPixbufModule *module)
{
  glycin_fill_vtable (module);

  module->save = gdk_pixbuf__png_image_save;
  module->save_to_callback = gdk_pixbuf__png_image_save_to_callback;
  module->is_save_option_supported = gdk_pixbuf__png_is_save_option_supported;
}

MODULE_ENTRY (fill_info) (GdkPixbufFormat *info)
{
  static const GdkPixbufModulePattern signature[] = {
    { "\x89PNG\r\n\x1a\x0a", NULL, 100 },
    { NULL, NULL, 0 }
  };
  static const gchar *mime_types[] = {
    "image/png",
    NULL
  };
  static const gchar *extensions[] = {
    "png",
    NULL
  };

  info->name = "png";
  info->signature = (GdkPixbufModulePattern *) signature;
  info->description = "PNG";
  info->mime_types = (gchar **) mime_types;
  info->extensions = (gchar **) extensions;
  info->flags = GDK_PIXBUF_FORMAT_WRITABLE | GDK_PIXBUF_FORMAT_THREADSAFE;
  info->license = "LGPL";
}
