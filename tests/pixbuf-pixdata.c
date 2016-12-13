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
#include <string.h>

static void
test_pixdata (void)
{
  const gchar *path;
  GError *error = NULL;
  GdkPixbuf *ref;

  ref = gdk_pixbuf_new_from_resource ("/test/resource/icc-profile.pixdata", &error);
  g_assert_no_error (error);
  g_object_unref (ref);

  path = g_test_get_filename (G_TEST_DIST, "test-image.pixdata", NULL);
  ref = gdk_pixbuf_new_from_file (path, &error);
  g_assert_error (error, GDK_PIXBUF_ERROR, GDK_PIXBUF_ERROR_UNKNOWN_TYPE);
  g_clear_error (&error);
  g_clear_object (&ref);
}

int
main (int argc, char **argv)
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/pixbuf/pixdata", test_pixdata);

  return g_test_run ();
}
