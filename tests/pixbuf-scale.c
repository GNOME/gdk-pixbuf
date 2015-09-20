/* -*- Mode: C; c-basic-offset: 2; -*- */
/* GdkPixbuf library - test loaders
 *
 * Copyright (C) 2013 Red Hat, Inc.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Matthias Clasen
 */

#include "config.h"
#include "gdk-pixbuf/gdk-pixbuf.h"
#include "test-common.h"

static void
test_scale (gconstpointer data)
{
  const gchar *filename = data;
  const gchar *path;
  GError *error = NULL;
  GdkPixbuf *ref;
  GdkPixbuf *pixbuf;
  gint width, height;

  if (!format_supported (filename))
    {
      g_test_skip ("format not supported");
      return;
    }

  path = g_test_get_filename (G_TEST_DIST, filename, NULL);
  ref = gdk_pixbuf_new_from_file (path, &error);
  g_assert_no_error (error);

  width = gdk_pixbuf_get_width (ref);
  height = gdk_pixbuf_get_height (ref);

  pixbuf = gdk_pixbuf_new_from_file_at_scale (path, 2 * width, 3 * height, FALSE, &error);
  g_assert_no_error (error);

  g_assert_cmpint (gdk_pixbuf_get_width (pixbuf), ==, 2 * width);
  g_assert_cmpint (gdk_pixbuf_get_height (pixbuf), ==, 3 * height);
  
  g_object_unref (pixbuf);

  pixbuf = gdk_pixbuf_new_from_file_at_scale (path, 4 * width, 2 * height, TRUE, &error);
  g_assert_no_error (error);

  g_assert_cmpint (gdk_pixbuf_get_width (pixbuf), ==, 2 * width);
  g_assert_cmpint (gdk_pixbuf_get_height (pixbuf), ==, 2 * height);
  
  g_object_unref (pixbuf);

  g_object_unref (ref);
}

static void
test_scale_down (gconstpointer data)
{
  const gchar *filename = data;
  const gchar *path;
  GError *error = NULL;
  GdkPixbuf *ref;
  GdkPixbuf *pixbuf;
  gint width, height;

  if (!format_supported (filename))
    {
      g_test_skip ("format not supported");
      return;
    }

  path = g_test_get_filename (G_TEST_DIST, filename, NULL);
  ref = gdk_pixbuf_new_from_file (path, &error);

  if (skip_if_insufficient_memory (&error))
    return;
  g_assert_no_error (error);

  width = gdk_pixbuf_get_width (ref);
  height = gdk_pixbuf_get_height (ref);

  pixbuf = gdk_pixbuf_scale_simple (ref, width / 10, height / 10, GDK_INTERP_BILINEAR);
  g_assert (pixbuf != NULL);

  g_object_unref (ref);
}

static void
test_add_alpha (gconstpointer data)
{
  const gchar *filename = data;
  const gchar *path;
  GError *error = NULL;
  GdkPixbuf *ref;
  GdkPixbuf *pixbuf;

  if (!format_supported (filename))
    {
      g_test_skip ("format not supported");
      return;
    }

  path = g_test_get_filename (G_TEST_DIST, filename, NULL);
  ref = gdk_pixbuf_new_from_file (path, &error);

  if (skip_if_insufficient_memory (&error))
    return;
  g_assert_no_error (error);

  pixbuf = gdk_pixbuf_add_alpha (ref, FALSE, 0, 0, 0);

  if (pixbuf == NULL)
    {
      g_test_skip ("Couldn't add alpha to the image - your system probably lacks sufficient memory.");
      g_object_unref (ref);
      return;
    }

  g_object_unref (pixbuf);

  pixbuf = gdk_pixbuf_add_alpha (ref, TRUE, 0, 0, 255);
  g_assert (pixbuf != NULL);
  g_object_unref (pixbuf);

  g_object_unref (ref);
}

static void
test_rotate (gconstpointer data)
{
  const gchar *filename = data;
  const gchar *path;
  GError *error = NULL;
  GdkPixbuf *ref;
  GdkPixbuf *pixbuf;

  if (!format_supported (filename))
    {
      g_test_skip ("format not supported");
      return;
    }

  path = g_test_get_filename (G_TEST_DIST, filename, NULL);
  ref = gdk_pixbuf_new_from_file (path, &error);

  if (skip_if_insufficient_memory (&error))
    return;
  g_assert_no_error (error);

  pixbuf = gdk_pixbuf_rotate_simple (ref, GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);

  if (pixbuf == NULL)
    g_test_skip ("Couldn't rotate the image - your system probably lacks sufficient memory.");
  else
    g_object_unref (pixbuf);

  g_object_unref (ref);
}

int
main (int argc, char **argv)
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_data_func ("/pixbuf/scale/png", "test-images/valid_png_test", test_scale);
  g_test_add_data_func ("/pixbuf/scale/bmp", "test-images/valid_bmp_test", test_scale);
  g_test_add_data_func ("/pixbuf/scale/gif", "test-images/valid_gif_test", test_scale);
  g_test_add_data_func ("/pixbuf/scale/jpeg", "test-images/valid_jpeg_test", test_scale);
  g_test_add_data_func ("/pixbuf/scale/ras", "test-images/valid_ras_test", test_scale);
  g_test_add_data_func ("/pixbuf/scale/tga", "test-images/valid_tga_test", test_scale);
  g_test_add_data_func ("/pixbuf/scale/xpm", "test-images/valid_xpm_test", test_scale);
  g_test_add_data_func ("/pixbuf/scale/xbm", "test-images/valid.xbm", test_scale);
  g_test_add_data_func ("/pixbuf/scale/png/large", "large.png", test_scale_down);
  g_test_add_data_func ("/pixbuf/scale/jpeg/large", "large.jpg", test_scale_down);
  g_test_add_data_func ("/pixbuf/add-alpha/large", "large.png", test_add_alpha);
  g_test_add_data_func ("/pixbuf/rotate/large", "large.png", test_rotate);

  return g_test_run ();
}
