/* GdkPixbuf library - Glycin image loader
 *
 * Copyright (C) 2026 Red Hat, Inc.
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
#include <glycin.h>

#ifndef INCLUDE_glycin
#define MODULE_ENTRY(function) G_MODULE_EXPORT void function
#else
#define MODULE_ENTRY(function) void _gdk_pixbuf__glycin_xpm_ ## function
#endif

static GdkPixbuf *
gdk_pixbuf__glycin_xpm_load_xpm_data (const gchar **data)
{
        int w, h, n_col, cpp, x_hot, y_hot, ii, n_rows;
        GString *xpm;
        GdkPixbuf *res;
        GBytes *bytes;
        GlyLoader *loader;

        if (data == NULL || data[0] == NULL)
                return NULL;

        /* while the passed-in data should be NULL-terminated, it most
           often is not. The old loader did stop on NULL or when read
           enough lines, thus do the same, to avoid breakage of
           the old code */
        ii = sscanf (data[0], "%d %d %d %d %d %d", &w, &h, &n_col, &cpp, &x_hot, &y_hot);

        if (ii != 4 && ii != 6)
                return NULL;

        /* construct XPM-like file content, to be passed into glycin */
        xpm = g_string_new ("/* XPM */\n");
        g_string_append (xpm, "static char *xpm_file[] = {\n");
        n_rows = 1 + n_col + h;
        for (ii = 0; data[ii] != NULL && ii < n_rows; ii++) {
                gboolean has_more_data = data[ii + 1] != NULL && ii + 1 < n_rows;
                const gchar *str = data[ii];

                g_string_append_c (xpm, '\"');
                if (strchr (str, '\"') || strchr (str, '\\')) {
                        int jj;
                        for (jj = 0; str[jj]; jj++) {
                                if (str[jj] == '\"' || str[jj] == '\\')
                                        g_string_append_c (xpm, '\\');
                                g_string_append_c (xpm, str[jj]);
                        }
                } else {
                        g_string_append (xpm, str);
                }
                g_string_append_c (xpm, '\"');
                g_string_append (xpm, has_more_data ? ",\n" : "};\n");
        }

        bytes = g_bytes_new_static (xpm->str, xpm->len);
        loader = gly_loader_new_for_bytes (bytes);
        g_bytes_unref (bytes);

        res = glycin_load_pixbuf (loader, NULL, NULL, NULL, NULL);

        g_string_free (xpm, TRUE);
        g_clear_object (&loader);

        return res;
}

MODULE_ENTRY (fill_vtable) (GdkPixbufModule *module)
{
  glycin_fill_vtable (module);
  module->load_xpm_data = gdk_pixbuf__glycin_xpm_load_xpm_data;
}

MODULE_ENTRY (fill_info) (GdkPixbufFormat *info)
{
  static const GdkPixbufModulePattern signature[] = {
    { "/* XPM */", NULL, 100 },
    { NULL, NULL, 0 }
  };
  static const gchar *mime_types[] = {
    "image/x-xpixmap",
    NULL
  };
  static const gchar *extensions[] = {
    "xpm",
    NULL
  };

  info->name = "xpm";
  info->signature = (GdkPixbufModulePattern *) signature;
  info->description = "XPM";
  info->mime_types = (gchar **) mime_types;
  info->extensions = (gchar **) extensions;
  info->flags = GDK_PIXBUF_FORMAT_THREADSAFE;
  info->license = "LGPL";
}
