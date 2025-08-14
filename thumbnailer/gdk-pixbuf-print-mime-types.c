/*
 * Copyright (C) 2016 Bastien Nocera <hadess@hadess.net>
 *
 * Authors: Bastien Nocera <hadess@hadess.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

#include <gdk-pixbuf/gdk-pixbuf.h>

int main (int argc, char **argv)
{
	GSList *formats;
	GSList *all_mime_types = NULL;
	GString *s;

	formats = gdk_pixbuf_get_formats ();
	for (GSList *l = formats; l != NULL; l = l->next) {
		GdkPixbufFormat *format = l->data;
		char **mime_types;

		mime_types = gdk_pixbuf_format_get_mime_types (format);
		for (guint i = 0; mime_types[i] != NULL; i++)
			all_mime_types = g_slist_prepend (all_mime_types, g_strdup (mime_types[i]));
		g_strfreev (mime_types);
	}
	g_slist_free (formats);

	all_mime_types = g_slist_sort (all_mime_types, (GCompareFunc)strcmp);

	s = g_string_new (NULL);
	for (GSList *l = all_mime_types; l != NULL; l = l->next) {
		g_string_append (s, l->data);
		g_string_append (s, ";");
	}
	g_slist_free_full (all_mime_types, g_free);

	g_print ("%s", s->str);
	g_string_free (s, TRUE);

	return 0;
}
