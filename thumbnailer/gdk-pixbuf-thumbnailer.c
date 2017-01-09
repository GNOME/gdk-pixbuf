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

#include <string.h>
#include <glib.h>

#include "gnome-thumbnailer-skeleton.h"

typedef struct {
    gint size;
    gint input_width;
    gint input_height;
} SizePrepareContext;

#define LOAD_BUFFER_SIZE 65536

static void
size_prepared_cb (GdkPixbufLoader *loader, 
		  int              width,
		  int              height,
		  gpointer         data)
{
  SizePrepareContext *info = data;
  
  g_return_if_fail (width > 0 && height > 0);
  
  info->input_width = width;
  info->input_height = height;
  
  if (width < info->size && height < info->size) return;
  if (info->size <= 0) return;
  
  if (height > width) {
    width = 0.5 + (double)width * (double)info->size / (double)height;
    height = info->size;
  } else {
    height = 0.5 + (double)height * (double)info->size / (double)width;
    width = info->size;
  }
  
  gdk_pixbuf_loader_set_size (loader, width, height);
}

static GdkPixbufLoader *
create_loader (GFile        *file,
               const guchar *data,
               gsize         size)
{
  GdkPixbufLoader *loader;
  GError *error = NULL;
  char *mime_type;
  char *filename;

  loader = NULL;

  /* need to specify the type here because the gdk_pixbuf_loader_write
     doesn't have access to the filename in order to correct detect
     the image type. */
  filename = g_file_get_basename (file);
  mime_type = g_content_type_guess (filename, data, size, NULL);
  g_free (filename);

  if (mime_type != NULL) {
    loader = gdk_pixbuf_loader_new_with_mime_type (mime_type, &error);
  }

  if (loader == NULL) {
    g_debug ("Unable to create loader for mime type %s: %s", mime_type,
             (error != NULL) ? error->message : "(null)");
    g_clear_error (&error);
    loader = gdk_pixbuf_loader_new ();
  }
  g_free (mime_type);

  return loader;
}

static GdkPixbuf *
_gdk_pixbuf_new_from_uri_at_scale (const char  *uri,
				   gint         size,
				   GError     **error)
{
    gboolean result;
    guchar buffer[LOAD_BUFFER_SIZE];
    gssize bytes_read;
    GdkPixbufLoader *loader = NULL;
    GdkPixbuf *pixbuf;	
    GdkPixbufAnimation *animation;
    GdkPixbufAnimationIter *iter;
    gboolean has_frame;
    SizePrepareContext info;
    GFile *file;
    GInputStream *input_stream;

    g_return_val_if_fail (uri != NULL, NULL);

    file = g_file_new_for_uri (uri);

    input_stream = G_INPUT_STREAM (g_file_read (file, NULL, error));
    if (input_stream == NULL) {
        g_object_unref (file);
        return NULL;
    }

    has_frame = FALSE;

    result = FALSE;
    while (!has_frame) {

	bytes_read = g_input_stream_read (input_stream,
					  buffer,
					  sizeof (buffer),
					  NULL,
					  error);
        if (bytes_read == -1) {
            break;
        }
	result = TRUE;
	if (bytes_read == 0) {
	    break;
	}

        if (loader == NULL) {
            loader = create_loader (file, buffer, bytes_read);
            if (1 <= size) {
              info.size = size;
              info.input_width = info.input_height = 0;
              g_signal_connect (loader, "size-prepared", G_CALLBACK (size_prepared_cb), &info);
            }
            g_assert (loader != NULL);
        }

	if (!gdk_pixbuf_loader_write (loader,
				      (unsigned char *)buffer,
				      bytes_read,
				      error)) {
	    result = FALSE;
	    break;
	}

	animation = gdk_pixbuf_loader_get_animation (loader);
	if (animation) {
		iter = gdk_pixbuf_animation_get_iter (animation, NULL);
		if (!gdk_pixbuf_animation_iter_on_currently_loading_frame (iter)) {
			has_frame = TRUE;
		}
		g_object_unref (iter);
	}
    }

    if (loader == NULL) {
        /* This can happen if the above loop was exited due to the
         * g_input_stream_read() call failing. */
        result = FALSE;
    } else if (*error != NULL) {
        gdk_pixbuf_loader_close (loader, NULL);
        result = FALSE;
    } else if (gdk_pixbuf_loader_close (loader, error) == FALSE) {
        if (!g_error_matches (*error, GDK_PIXBUF_ERROR, GDK_PIXBUF_ERROR_INCOMPLETE_ANIMATION))
          result = FALSE;
        else
          g_clear_error (error);
    }

    if (!result) {
        g_clear_object (&loader);
	g_input_stream_close (input_stream, NULL, NULL);
	g_object_unref (input_stream);
	g_object_unref (file);
	return NULL;
    }

    g_input_stream_close (input_stream, NULL, NULL);
    g_object_unref (input_stream);
    g_object_unref (file);

    pixbuf = gdk_pixbuf_loader_get_pixbuf (loader);
    if (pixbuf != NULL) {
	g_object_ref (G_OBJECT (pixbuf));
	g_object_set_data (G_OBJECT (pixbuf), "gnome-original-width",
			   GINT_TO_POINTER (info.input_width));
	g_object_set_data (G_OBJECT (pixbuf), "gnome-original-height",
			   GINT_TO_POINTER (info.input_height));
    }
    g_object_unref (G_OBJECT (loader));

    return pixbuf;
}

GdkPixbuf *
file_to_pixbuf (const char  *path,
		guint        destination_size,
	        GError     **error)
{
	GdkPixbuf *pixbuf, *tmp_pixbuf;
	GFile *file;
	char *uri;
	int original_width, original_height;

	file = g_file_new_for_path (path);
	uri = g_file_get_uri (file);
	pixbuf = _gdk_pixbuf_new_from_uri_at_scale (uri, destination_size, error);
	if (pixbuf == NULL)
		return NULL;

	tmp_pixbuf = gdk_pixbuf_apply_embedded_orientation (pixbuf);
	gdk_pixbuf_copy_options (pixbuf, tmp_pixbuf);
	gdk_pixbuf_remove_option (tmp_pixbuf, "orientation");
	g_object_unref (pixbuf);
	pixbuf = tmp_pixbuf;

        original_width = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (pixbuf),
                                                             "gnome-original-width"));
        original_height = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (pixbuf),
                                                              "gnome-original-height"));

	if (original_width > 0 && original_height > 0) {
		char *tmp;

		tmp = g_strdup_printf ("%d", original_width);
		gdk_pixbuf_set_option (pixbuf, "tEXt::Thumb::Image::Width", tmp);
		g_free (tmp);

		tmp = g_strdup_printf ("%d", original_height);
		gdk_pixbuf_set_option (pixbuf, "tEXt::Thumb::Image::Height", tmp);
		g_free (tmp);
	}

	return pixbuf;
}
