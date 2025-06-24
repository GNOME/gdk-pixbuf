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

#include "io-android-utils.h"


#define MODULE_ENTRY(function) void _gdk_pixbuf__android_png_ ## function

static gboolean
gdk_pixbuf__android_save_image_png (GdkPixbufSaveFunc save_func, gpointer user_data,
                                    GdkPixbuf *pixbuf,
                                    gchar **option_keys, gchar **option_values,
                                    GError **error)
{
  return gdk_pixbuf__android_save_image (save_func, user_data,
                                         pixbuf, ANDROID_BITMAP_COMPRESS_FORMAT_PNG,
                                         error);
}

MODULE_ENTRY (fill_vtable) (GdkPixbufModule *module)
{
  gdk_pixbuf__android_fill_vtable (module);

  module->save_to_callback = gdk_pixbuf__android_save_image_png;
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
  info->license = "Apache-2.0";
}
