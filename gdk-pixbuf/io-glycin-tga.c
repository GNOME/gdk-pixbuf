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


#ifndef INCLUDE_glycin
#define MODULE_ENTRY(function) G_MODULE_EXPORT void function
#else
#define MODULE_ENTRY(function) void _gdk_pixbuf__glycin_tga_ ## function
#endif

MODULE_ENTRY (fill_vtable) (GdkPixbufModule *module)
{
  glycin_fill_vtable (module);
}

MODULE_ENTRY (fill_info) (GdkPixbufFormat *info)
{
  static const GdkPixbufModulePattern signature[] = {
    { " \x1\x1", "x  ", 100 },
    { " \x1\x9", "x  ", 100 },
    { "  \x2", "xz ",  99 }, /* only 99 since .CUR also matches this */
    { "  \x3", "xz ", 100 },
    { "  \xa", "xz ", 100 },
    { "  \xb", "xz ", 100 },
    { NULL, NULL, 0 }
  };
  static const gchar *mime_types[] = {
    "image/x-tga",
    NULL
  };
  static const gchar *extensions[] = {
    "tga",
    "targa",
    NULL
  };

  info->name = "tga";
  info->signature = (GdkPixbufModulePattern *) signature;
  info->description = "Targa";
  info->mime_types = (gchar **) mime_types;
  info->extensions = (gchar **) extensions;
  info->flags = GDK_PIXBUF_FORMAT_THREADSAFE;
  info->license = "LGPL";
}
