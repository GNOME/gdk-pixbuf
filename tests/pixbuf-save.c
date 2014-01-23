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

#include "gdk-pixbuf/gdk-pixbuf.h"
#include "test-common.h"
#include <string.h>

#define compare_option(p1, p2, key) \
  g_strcmp0 (gdk_pixbuf_get_option (p1, key), gdk_pixbuf_get_option (p2, key))

static gboolean
pixdata_equal (GdkPixbuf *p1, GdkPixbuf *p2)
{
  if (gdk_pixbuf_get_colorspace (p1) != gdk_pixbuf_get_colorspace (p2))
    return FALSE;
  if (gdk_pixbuf_get_n_channels (p1) != gdk_pixbuf_get_n_channels (p2))
    return FALSE;
  if (gdk_pixbuf_get_bits_per_sample (p1) != gdk_pixbuf_get_bits_per_sample (p2))
    return FALSE;
  if (gdk_pixbuf_get_width (p1) != gdk_pixbuf_get_width (p2))
    return FALSE;
  if (gdk_pixbuf_get_height (p1) != gdk_pixbuf_get_height (p2))
    return FALSE;
  if (gdk_pixbuf_get_rowstride (p1) != gdk_pixbuf_get_rowstride (p2))
    return FALSE;
  if (memcmp (gdk_pixbuf_get_pixels (p1), gdk_pixbuf_get_pixels (p2),
          gdk_pixbuf_get_byte_length (p1)) != 0)
    return FALSE;

  return TRUE;
}

static gboolean
pixbuf_equal (GdkPixbuf *p1, GdkPixbuf *p2)
{
  if (!pixdata_equal (p1, p2))
    return FALSE;
  if (compare_option (p1, p2, "Title") != 0)
    return FALSE;
  if (compare_option (p1, p2, "Artist") != 0)
    return FALSE;
  if (compare_option (p1, p2, "x_hot") != 0)
    return FALSE;
  if (compare_option (p1, p2, "y_hot") != 0)
    return FALSE;
  if (compare_option (p1, p2, "orientation") != 0)
    return FALSE;
  if (compare_option (p1, p2, "icc-profile") != 0)
    return FALSE;

  return TRUE;
}

static void
test_save_roundtrip (void)
{
  GError *error = NULL;
  GdkPixbuf *ref;
  GdkPixbuf *pixbuf;

  if (!format_supported ("png"))
    {
      g_test_skip ("format not supported");
      return;
    }

  ref = gdk_pixbuf_new_from_file (g_test_get_filename (G_TEST_DIST, "test-image.png", NULL), &error);
  g_assert_no_error (error);

  gdk_pixbuf_save (ref, "pixbuf-save-roundtrip", "png", &error, NULL);
  g_assert_no_error (error);

  pixbuf = gdk_pixbuf_new_from_file ("pixbuf-save-roundtrip", &error);
  g_assert_no_error (error);

  g_assert (pixbuf_equal (pixbuf, ref));

  g_object_unref (pixbuf);
  g_object_unref (ref);
}

static void
test_save_options (void)
{
  GdkPixbuf *ref;
  GdkPixbuf *pixbuf;
  GError *error = NULL;

  if (!format_supported ("png"))
    {
      g_test_skip ("format not supported");
      return;
    }

  ref = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, 10, 10);
  gdk_pixbuf_fill (ref, 0xff00ff00);

  gdk_pixbuf_save (ref, "pixbuf-save-options", "png", &error,
                   "tEXt::option1", "Some text to transport via option",
                   "tEXt::long-option-name123456789123456789123456789", "",
#ifdef PNG_iTXt_SUPPORTED
                   "tEXt::3", "αβγδ",
#endif
                   NULL);
  g_assert_no_error (error);

  pixbuf = gdk_pixbuf_new_from_file ("pixbuf-save-options", &error);
  g_assert_no_error (error);

  g_assert_cmpstr (gdk_pixbuf_get_option (pixbuf, "tEXt::option1"), ==, "Some text to transport via option");
  g_assert_cmpstr (gdk_pixbuf_get_option (pixbuf, "tEXt::long-option-name123456789123456789123456789"), ==, "");
#ifdef PNG_iTXt_SUPPORTED
  g_assert_cmpstr (gdk_pixbuf_get_option (pixbuf, "tEXt::3"), ==, "αβγδ");
#endif

  g_object_unref (pixbuf);
  g_object_unref (ref);
}

int
main (int argc, char **argv)
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/pixbuf/save/roundtrip", test_save_roundtrip);
  g_test_add_func ("/pixbuf/save/options", test_save_options);

  return g_test_run ();
}
