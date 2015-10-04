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

  *pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, w, h);
  g_assert (*pixbuf != NULL);
}

static void
loader_area_prepared (GdkPixbufLoader  *loader,
                      int               w,
                      int               h,
                      GdkPixbuf       **pixbuf)
{
  g_assert (*pixbuf != NULL);

  if (gdk_pixbuf_get_has_alpha (gdk_pixbuf_loader_get_pixbuf (loader)))
    {
      GdkPixbuf *alpha = gdk_pixbuf_add_alpha (*pixbuf, FALSE, 0, 0, 0);

      g_object_unref (*pixbuf);
      *pixbuf = alpha;
    }

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

static char *
make_ref_filename (const char *filename)
{
  return g_strconcat (filename, ".ref.png", NULL);
}

static gboolean
is_ref_filename (const char *filename)
{
  return g_str_has_suffix (filename, ".ref.png");
}

static void
test_reftest_success (gconstpointer file)
{
  GdkPixbufLoader *loader;
  GdkPixbuf *reference, *loaded = NULL;
  GError *error = NULL;
  const char *filename;
  char *ref_filename;
  guchar *contents;
  gsize i, contents_length;
  gboolean success;

  filename = file;
  if (is_ref_filename (filename))
    return;

  ref_filename = make_ref_filename (filename);
  reference = gdk_pixbuf_new_from_file (ref_filename, &error);
  g_assert_no_error (error);
  g_assert (reference != NULL);

  loader = gdk_pixbuf_loader_new ();
  g_assert (loader != NULL);
  g_signal_connect (loader, "size-prepared", G_CALLBACK (loader_size_prepared), &loaded);
  g_signal_connect (loader, "area-prepared", G_CALLBACK (loader_area_prepared), &loaded);
  g_signal_connect (loader, "area-updated", G_CALLBACK (loader_area_updated), &loaded);

  success = g_file_get_contents (filename, (gchar **) &contents, &contents_length, &error);
  g_assert_no_error (error);
  g_assert (success);

  for (i = 0; i < contents_length; i++)
    {
      success = gdk_pixbuf_loader_write (loader, &contents[i], 1, &error);
      g_assert_no_error (error);
      g_assert (success);
    }
  
  success = gdk_pixbuf_loader_close (loader, &error);
  g_assert_no_error (error);
  g_assert (success);

  g_assert (loaded != NULL);

  success = pixdata_equal (reference, loaded, &error);
  g_assert_no_error (error);
  g_assert (success);

  g_free (contents);
  g_object_unref (loaded);
  g_object_unref (loader);
  g_object_unref (reference);
  g_free (ref_filename);
}

int
main (int argc, char **argv)
{
  gchar *tga_test_images;

  g_test_init (&argc, &argv, NULL);

  tga_test_images = g_build_filename (g_test_get_dir (G_TEST_DIST), "test-images/tga", NULL);
  add_test_for_all_images ("/pixbuf/reftest/success", tga_test_images, test_reftest_success);
  g_free (tga_test_images);

  return g_test_run ();
}
