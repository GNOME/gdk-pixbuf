#include <gdk-pixbuf/gdk-pixbuf.h>

int main (int argc, char **argv)
{
	GSList *formats, *l;
	GString *s;

	formats = gdk_pixbuf_get_formats ();
	s = g_string_new (NULL);
	for (l = formats; l != NULL; l = l->next) {
		GdkPixbufFormat *format = l->data;
		char **mime_types;
		guint i;

		mime_types = gdk_pixbuf_format_get_mime_types (format);
		for (i = 0; mime_types[i] != NULL; i++) {
			g_string_append (s, mime_types[i]);
			g_string_append (s, ";");
		}

		g_strfreev (mime_types);
	}
	g_slist_free (formats);

	g_print ("%s", s->str);
	g_string_free (s, TRUE);

	return 0;
}
