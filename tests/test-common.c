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
  const gchar *name = NULL;
  GSList *formats, *l;
  gboolean retval;
  const gchar *names[] = { "png", "jpeg", "bmp", "gif", "ras",
    "tga", "xpm", "xbm", "ico", "tiff" };
  gint i;

  for (i = 0; i < G_N_ELEMENTS (names); i++)
    {
      if (strstr (filename, names[i]))
        {
          name = names[i];
          break;
        }
    }
  if (name == NULL)
    return FALSE;

  retval = FALSE;
  formats = gdk_pixbuf_get_formats ();
  for (l = formats; l; l = l->next)
    {
      GdkPixbufFormat *format = l->data;

      if (g_str_equal (gdk_pixbuf_format_get_name (format), name))
        {
          retval = TRUE;
          break;
        }
    }
  g_slist_free (formats);

  return retval;
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
      g_set_error_literal (error, GDK_PIXBUF_ERROR, 0, "Colorspaces differ");
      return FALSE;
    }

  if (gdk_pixbuf_get_n_channels (test) != gdk_pixbuf_get_n_channels (ref))
    {
      g_set_error_literal (error, GDK_PIXBUF_ERROR, 0, "Channels differ");
      return FALSE;
    }

  if (gdk_pixbuf_get_bits_per_sample (test) != gdk_pixbuf_get_bits_per_sample (ref))
    {
      g_set_error_literal (error, GDK_PIXBUF_ERROR, 0, "BPP differ");
      return FALSE;
    }

  if (gdk_pixbuf_get_width (test) != gdk_pixbuf_get_width (ref))
    {
      g_set_error_literal (error, GDK_PIXBUF_ERROR, 0, "Widths differ");
      return FALSE;
    }

  if (gdk_pixbuf_get_height (test) != gdk_pixbuf_get_height (ref))
    {
      g_set_error_literal (error, GDK_PIXBUF_ERROR, 0, "Heights differ");
      return FALSE;
    }

  if (gdk_pixbuf_get_rowstride (test) != gdk_pixbuf_get_rowstride (ref))
    {
      g_set_error_literal (error, GDK_PIXBUF_ERROR, 0, "Rowstrides differ");
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
                  g_set_error (error, GDK_PIXBUF_ERROR, 0, "Data differ at %ux%u", x, y);
                  return FALSE;
                }
            }
          test_pixels += rowstride;
          ref_pixels += rowstride;
        }
    }

  return TRUE;
}

void
add_test_for_all_images (const gchar   *prefix,
                         const gchar   *path,
                         GTestDataFunc  test_func)
{
  GDir *dir;
  const gchar *name;

  dir = g_dir_open (path, 0, NULL);
  while ((name = g_dir_read_name (dir)) != NULL)
    {
      gchar *test_path;
      gchar *dir_path;

      test_path = g_strconcat (prefix, "/", name, NULL);
      dir_path = g_strconcat (path, "/", name, NULL);
      if (g_file_test (dir_path, G_FILE_TEST_IS_DIR))
        add_test_for_all_images (test_path, dir_path, test_func);
      else
        g_test_add_data_func_full (test_path, g_strdup (dir_path), test_func, g_free);
      g_free (test_path);
      g_free (dir_path);
    }
  g_dir_close (dir);
}
