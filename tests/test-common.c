/* -*- Mode: C; c-basic-offset: 2; -*- */
/* GdkPixbuf library - test loaders
 *
 * Copyright (C) 2014 Red Hat, Inc.
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 * Author: Matthias Clasen
 */

#include "config.h"
#include "test-common.h"
#include "gdk-pixbuf/gdk-pixbuf.h"

#include <string.h>

gboolean
format_supported (const gchar *filename)
{
  GSList *formats, *l;
  gboolean retval;

  retval = FALSE;
  formats = gdk_pixbuf_get_formats ();
  for (l = formats; l; l = l->next)
    {
      GdkPixbufFormat *format = l->data;
      char **extensions = gdk_pixbuf_format_get_extensions (format);
      gint i;

      for (i = 0; extensions[i]; i++)
        {
          if (g_str_has_suffix (filename, extensions[i]))
            {
              retval = TRUE;
              break;
            }
        }

      g_free (extensions);
      if (retval)
        break;
    }
  g_slist_free (formats);

  return retval;
}

gboolean
file_supported (GFile *file)
{
  char *uri;
  gboolean result;

  uri = g_file_get_uri (file);

  result = format_supported (uri);

  g_free (uri);

  return result;
}

gboolean
skip_if_insufficient_memory (GError **err)
{
  if (*err && g_error_matches (*err, GDK_PIXBUF_ERROR, GDK_PIXBUF_ERROR_INSUFFICIENT_MEMORY))
  {
      g_test_skip ((*err)->message);
      g_error_free (*err);
      *err = NULL;
      return TRUE;
  }

  return FALSE;
}

gboolean
pixdata_equal (GdkPixbuf  *test,
               GdkPixbuf  *ref,
               GError    **error)
{
  if (gdk_pixbuf_get_colorspace (test) != gdk_pixbuf_get_colorspace (ref))
    {
      g_set_error (error, GDK_PIXBUF_ERROR, 0, "Image colorspace is %d but should be %d",
                   gdk_pixbuf_get_colorspace (test), gdk_pixbuf_get_colorspace (ref));
      return FALSE;
    }

  if (gdk_pixbuf_get_n_channels (test) != gdk_pixbuf_get_n_channels (ref))
    {
      g_set_error (error, GDK_PIXBUF_ERROR, 0,
                   "has %u channels but should have %u",
                   gdk_pixbuf_get_n_channels (test), gdk_pixbuf_get_n_channels (ref));
      return FALSE;
    }

  if (gdk_pixbuf_get_bits_per_sample (test) != gdk_pixbuf_get_bits_per_sample (ref))
    {
      g_set_error (error, GDK_PIXBUF_ERROR, 0,
                   "Image is %u bits per sample but should be %u bits per sample",
                   gdk_pixbuf_get_bits_per_sample (test), gdk_pixbuf_get_bits_per_sample (ref));
      return FALSE;
    }

  if (gdk_pixbuf_get_width (test) != gdk_pixbuf_get_width (ref) ||
      gdk_pixbuf_get_height (test) != gdk_pixbuf_get_height (ref))
    {
      g_set_error (error, GDK_PIXBUF_ERROR, 0,
                   "Image size is %dx%d but should be %dx%d",
                   gdk_pixbuf_get_width (test), gdk_pixbuf_get_height (test),
                   gdk_pixbuf_get_width (ref), gdk_pixbuf_get_height (ref));
      return FALSE;
    }

  if (gdk_pixbuf_get_rowstride (test) != gdk_pixbuf_get_rowstride (ref))
    {
      g_set_error (error, GDK_PIXBUF_ERROR, 0,
                   "Image rowstrides is %u bytes but should be %u bytes",
                   gdk_pixbuf_get_rowstride (test), gdk_pixbuf_get_rowstride (ref));
      return FALSE;
    }

  if (memcmp (gdk_pixbuf_get_pixels (test), gdk_pixbuf_get_pixels (ref),
          gdk_pixbuf_get_byte_length (test)) != 0)
    {
      guint x, y, width, height, n_channels, rowstride;
      const guchar *test_pixels, *ref_pixels;

      rowstride = gdk_pixbuf_get_rowstride (test);
      n_channels = gdk_pixbuf_get_n_channels (test);
      width = gdk_pixbuf_get_width (test);
      height = gdk_pixbuf_get_height (test);
      test_pixels = gdk_pixbuf_get_pixels (test);
      ref_pixels = gdk_pixbuf_get_pixels (ref);

      for (y = 0; y < height; y++)
        {
          for (x = 0; x < width; x++)
            {
              if (memcmp (&test_pixels[x * n_channels], &ref_pixels[x * n_channels], n_channels) != 0)
                {
                  if (n_channels == 4)
                    {
                      g_set_error (error, GDK_PIXBUF_ERROR, 0, "Image data at %ux%u is #%02X%02X%02X%02X, but should be #%02X%02X%02X%02X",
                                   x, y,
                                   test_pixels[0], test_pixels[1], test_pixels[2], test_pixels[3],
                                   ref_pixels[0], ref_pixels[1], ref_pixels[2], ref_pixels[3]);
                    }
                  else if (n_channels == 3)
                    {
                      g_set_error (error, GDK_PIXBUF_ERROR, 0, "Image data at %ux%u is #%02X%02X%02X, but should be #%02X%02X%02X",
                                   x, y,
                                   test_pixels[0], test_pixels[1], test_pixels[2],
                                   ref_pixels[0], ref_pixels[1], ref_pixels[2]);
                    }
                  else
                    {
                      g_set_error (error, GDK_PIXBUF_ERROR, 0, "Image data differ at %ux%u", x, y);
                    }
                  return FALSE;
                }
            }
          test_pixels += rowstride;
          ref_pixels += rowstride;
        }
    }

  return TRUE;
}

static int
compare_files (gconstpointer a, gconstpointer b)
{
  GFile *file1 = G_FILE (a);
  GFile *file2 = G_FILE (b);
  char *uri1, *uri2;
  int result;

  uri1 = g_file_get_uri (file1);
  uri2 = g_file_get_uri (file2);

  result = strcmp (uri1, uri2);

  g_free (uri1);
  g_free (uri2);

  return result;
}

void
add_test_for_all_images (const gchar   *prefix,
                         GFile         *base,
                         GFile         *file,
                         GTestDataFunc  test_func,
                         AddTestFunc    add_test_func)
{
  GFileEnumerator *enumerator;
  GFileInfo *info;
  GList *l, *files;
  GError *error = NULL;


  if (g_file_query_file_type (file, 0, NULL) != G_FILE_TYPE_DIRECTORY)
    {
      gchar *test_path;
      gchar *relative_path;

      if (base)
        relative_path = g_file_get_relative_path (base, file);
      else
        relative_path = g_file_get_path (file);

      test_path = g_strconcat (prefix, "/", relative_path, NULL);
      
      g_test_add_data_func_full (test_path, g_object_ref (file), test_func, g_object_unref);
      return;
    }


  enumerator = g_file_enumerate_children (file, G_FILE_ATTRIBUTE_STANDARD_NAME, 0, NULL, &error);
  g_assert_no_error (error);
  files = NULL;

  while ((info = g_file_enumerator_next_file (enumerator, NULL, &error)))
    {
      GFile *next_file = g_file_get_child (file, g_file_info_get_name (info));

      if (add_test_func == NULL || add_test_func (next_file))
        {
          files = g_list_prepend (files, g_object_ref (next_file));
        }

      g_object_unref (next_file);
      g_object_unref (info);
    }
  
  g_assert_no_error (error);
  g_object_unref (enumerator);

  files = g_list_sort (files, compare_files);

  for (l = files; l; l = l->next)
    {
      add_test_for_all_images (prefix, base, l->data, test_func, add_test_func);
    }

  g_list_free_full (files, g_object_unref);
}
