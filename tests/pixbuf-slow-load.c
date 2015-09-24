/* -*- Mode: C; c-basic-offset: 2; -*- */
/* GdkPixbuf library - test loaders
 *
 * Copyright (C) 2015 Red Hat, Inc.
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
 * Author: Benjamin Otte
 */

#include "config.h"

#include <gdk-pixbuf/gdk-pixbuf.h>

#include "test-common.h"

static void
loader_size_prepared (GdkPixbufLoader  *loader,
                      int               w,
                      int               h,
                      GdkPixbuf       **pixbuf)
{
  g_assert (*pixbuf == NULL);

  *pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, w, h);
  g_assert (*pixbuf != NULL);
}

static void
loader_area_updated (GdkPixbufLoader  *loader,
                     int               x,
                     int               y,
                     int               w,
                     int               h,
                     GdkPixbuf       **pixbuf)
{
  gdk_pixbuf_copy_area (gdk_pixbuf_loader_get_pixbuf (loader),
                        x, y,
                        w, h,
                        *pixbuf,
                        x, y);
}

static void
test_slow_load (gconstpointer file)
{
  GdkPixbufLoader *loader;
  GdkPixbuf *reference, *loaded = NULL;
  GError *error = NULL;
  const char *filename;
  guchar *contents;
  gsize i, contents_length;

  filename = file;
  reference = gdk_pixbuf_new_from_file (filename, &error);
  g_assert_no_error (error);
  g_assert (reference != NULL);
  if (!gdk_pixbuf_get_has_alpha (reference))
    {
      GdkPixbuf *tmp = gdk_pixbuf_add_alpha (reference, FALSE, 0, 0, 0);
      g_object_unref (reference);
      reference = tmp;
    }

  loader = gdk_pixbuf_loader_new_with_mime_type ("image/x-tga", &error);
  g_assert_no_error (error);
  g_assert (loader != NULL);
  g_signal_connect (loader, "size-prepared", G_CALLBACK (loader_size_prepared), &loaded);
  g_signal_connect (loader, "area-updated", G_CALLBACK (loader_area_updated), &loaded);

  if (!g_file_get_contents (filename, (gchar **) &contents, &contents_length, &error))
    g_assert_not_reached ();
  g_assert_no_error (error);

  for (i = 0; i < contents_length; i++)
    {
      if (!gdk_pixbuf_loader_write (loader, &contents[i], 1, &error))
        g_assert_not_reached();
      g_assert_no_error (error);
    }
  
  if (!gdk_pixbuf_loader_close (loader, &error))
    g_assert_not_reached ();
  g_assert_no_error (error);
  g_assert (loaded != NULL);

  g_assert (pixdata_equal (reference, loaded, &error));
  g_assert_no_error (error);

  g_free (contents);
  g_object_unref (loaded);
  g_object_unref (loader);
  g_object_unref (reference);
}

int
main (int argc, char **argv)
{
  gchar *tga_test_images;

  g_test_init (&argc, &argv, NULL);

  tga_test_images = g_build_filename (g_test_get_dir (G_TEST_DIST), "test-images/tga", NULL);
  add_test_for_all_images ("/pixbuf/slow-load", tga_test_images, test_slow_load);
  g_free (tga_test_images);

  return g_test_run ();
}
