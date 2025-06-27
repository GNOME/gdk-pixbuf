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
gdk_pixbuf__bmp_image_save (FILE       *f,
                            GdkPixbuf  *pixbuf,
                            char      **keys,
                            char      **values,
                            GError    **error)
{
  return glycin_image_save ("image/bmp", f, NULL, NULL,
                            pixbuf, NULL, NULL, NULL, -1, -1, error);
}

static gboolean
gdk_pixbuf__bmp_image_save_to_callback (GdkPixbufSaveFunc   save_func,
                                        gpointer            user_data,
                                        GdkPixbuf          *pixbuf,
                                        gchar             **keys,
                                        gchar             **values,
                                        GError            **error)
{
  return glycin_image_save ("image/bmp", NULL, save_func, user_data,
                            pixbuf, NULL, NULL, NULL, -1, -1, error);
}

static gboolean
gdk_pixbuf__bmp_is_save_option_supported (const gchar *option_key)
{
  return FALSE;
}

#ifndef INCLUDE_glycin
#define MODULE_ENTRY(function) G_MODULE_EXPORT void function
#else
#define MODULE_ENTRY(function) void _gdk_pixbuf__glycin_bmp_ ## function
#endif

MODULE_ENTRY (fill_vtable) (GdkPixbufModule *module)
{
  glycin_fill_vtable (module);

  module->save = gdk_pixbuf__bmp_image_save;
  module->save_to_callback = gdk_pixbuf__bmp_image_save_to_callback;
  module->is_save_option_supported = gdk_pixbuf__bmp_is_save_option_supported;
}

MODULE_ENTRY (fill_info) (GdkPixbufFormat *info)
{
  static const GdkPixbufModulePattern signature[] = {
    { "BM", NULL, 100 },
    { NULL, NULL, 0 }
  };
  static const gchar * mime_types[] = {
    "image/bmp",
    "image/x-bmp",
    "image/x-MS-bmp",
    NULL
  };
  static const gchar * extensions[] = {
    "bmp",
    NULL
  };

  info->name = "bmp";
  info->signature = (GdkPixbufModulePattern *) signature;
  info->description = "BMP";
  info->mime_types = (gchar **) mime_types;
  info->extensions = (gchar **) extensions;
  info->flags = GDK_PIXBUF_FORMAT_WRITABLE | GDK_PIXBUF_FORMAT_THREADSAFE;
  info->license = "LGPL";
}
