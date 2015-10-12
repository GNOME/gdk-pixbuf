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
test_fail (gconstpointer data)
{
  GdkPixbufLoader *loader;
  GError *error = NULL;
  GFile *file;
  guchar *contents;
  gsize i, contents_length;
  char *filename, *content_type, *mime_type;
  gboolean success;

  file = G_FILE (data);
  if (!file_supported (file))
    {
      g_test_skip ("format not supported");
      return;
    }

  filename = g_file_get_path (file);

  success = g_file_load_contents (file, NULL, (gchar **) &contents, &contents_length, NULL, &error);
  g_assert_no_error (error);
  g_assert (success);

  content_type = g_content_type_guess (filename, contents, contents_length, NULL);
  mime_type = g_content_type_get_mime_type (content_type);
  g_assert (mime_type);

  loader = gdk_pixbuf_loader_new_with_mime_type (mime_type, &error);
  g_assert_no_error (error);
  g_assert (loader != NULL);

  for (i = 0; i < contents_length; i++)
    {
      success = gdk_pixbuf_loader_write (loader, &contents[i], 1, &error);
      if (!success)
        {
          g_assert (error);
          g_clear_error (&error);
          goto out;
        }
      g_assert_no_error (error);
    }
  
  success = gdk_pixbuf_loader_close (loader, &error);
  g_assert (!success);
  g_assert (error);
  g_clear_error (&error);

out:
  g_free (mime_type);
  g_free (content_type);
  g_free (contents);
  g_object_unref (loader);
  g_free (filename);
}

int
main (int argc, char **argv)
{

  g_test_init (&argc, &argv, NULL);

  if (argc < 2)
    {
      GFile *dir;
      gchar *test_images;

      test_images = g_build_filename (g_test_get_dir (G_TEST_DIST), "test-images/fail", NULL);
      dir = g_file_new_for_path (test_images);
      
      add_test_for_all_images ("/pixbuf/fail", dir, dir, test_fail, NULL);

      g_object_unref (dir);
      g_free (test_images);
    }
  else
    {
      guint i;

      for (i = 1; i < argc; i++)
        {
          GFile *file = g_file_new_for_commandline_arg (argv[i]);

          add_test_for_all_images ("/pixbuf/fail", NULL, file, test_fail, NULL);

          g_object_unref (file);
        }
    }

  return g_test_run ();
}
