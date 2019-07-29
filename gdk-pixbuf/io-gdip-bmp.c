/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* GdkPixbuf library - Win32 GDI+ Pixbuf Loader
 *
 * Copyright (C) 2008 Dominic Lachowicz
 * Copyright (C) 2008 Alberto Ruiz
 *
 * Authors: Dominic Lachowicz <domlachowicz@gmail.com>
 *          Alberto Ruiz <aruiz@gnome.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more  * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "io-gdip-utils.h"

static gboolean
gdk_pixbuf__gdip_image_save_BMP_to_callback (GdkPixbufSaveFunc   save_func,
                                             gpointer            user_data,
                                             GdkPixbuf          *pixbuf,
                                             gchar             **keys,
                                             gchar             **values,
                                             GError            **error)
{
  return gdip_save_pixbuf (pixbuf, L"image/bmp", NULL, save_func, user_data, error);
}

static gboolean
gdk_pixbuf__gdip_image_save_BMP (FILE          *f,
                                 GdkPixbuf     *pixbuf,
                                 gchar        **keys,
                                 gchar        **values,
                                 GError       **error)
{
  return gdk_pixbuf__gdip_image_save_BMP_to_callback (gdip_save_to_file_callback, f, pixbuf, keys, values, error);
}

#ifndef INCLUDE_gdip_bmp
#define MODULE_ENTRY(function) G_MODULE_EXPORT void function
#else
#define MODULE_ENTRY(function) void _gdk_pixbuf__gdip_bmp_ ## function
#endif

MODULE_ENTRY (fill_vtable) (GdkPixbufModule *module)
{
  gdip_fill_vtable (module);

  module->save_to_callback = gdk_pixbuf__gdip_image_save_BMP_to_callback;
  module->save = gdk_pixbuf__gdip_image_save_BMP; /* for gtk < 2.14, you need to implement both. otherwise gdk-pixbuf-queryloaders fails */
}

MODULE_ENTRY (fill_info) (GdkPixbufFormat *info)
{
  static const GdkPixbufModulePattern signature[] = {
    { "BM", NULL, 100 }, /* BMP */
    { NULL, NULL, 0 }
  };

  static const gchar *mime_types[] = {
    "image/bmp",
    "image/x-bmp",
    "image/x-MS-bmp",
    NULL
  };

  static const gchar *extensions[] = {
    "bmp",
    NULL
  };

  info->name        = "bmp";
  info->signature   = (GdkPixbufModulePattern *) signature;
  info->description = NC_("image format", "BMP");
  info->mime_types  = (gchar **) mime_types;
  info->extensions  = (gchar **) extensions;
  info->flags       = GDK_PIXBUF_FORMAT_WRITABLE | GDK_PIXBUF_FORMAT_THREADSAFE;
  info->license     = "LGPL";
}
