/* GdkPixbuf library - Glycin image loader
 *
 * Copyright (C) 2025 Red Hat, Inc.
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
#define MODULE_ENTRY(function) void _gdk_pixbuf__glycin_svg_ ## function
#endif

MODULE_ENTRY (fill_vtable) (GdkPixbufModule *module)
{
  glycin_fill_vtable (module);
}

MODULE_ENTRY (fill_info) (GdkPixbufFormat *info)
{
  static const GdkPixbufModulePattern signature[] = {
    {  " <svg",  "*    ", 100 },
    {  " <!DOCTYPE svg",  "*             ", 100 },
    { NULL, NULL, 0 }
  };
  static const gchar * mime_types[] = {
    "image/svg+xml",
    "image/svg",
    "image/svg-xml",
    "image/vnd.adobe.svg+xml",
    "text/xml-svg",
    "image/svg+xml-compressed",
    NULL
  };
  static const gchar * extensions[] = {
    "svg",
    "svgz",
    "svg.gz",
    NULL
  };

  info->name = "svg";
  info->signature = (GdkPixbufModulePattern *) signature;
  info->description = "Scalable Vector Graphics";
  info->mime_types = (gchar **) mime_types;
  info->extensions = (gchar **) extensions;
  info->flags = GDK_PIXBUF_FORMAT_SCALABLE | GDK_PIXBUF_FORMAT_THREADSAFE;
  info->license = "LGPL";
}
