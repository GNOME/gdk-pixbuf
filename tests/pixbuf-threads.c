/* -*- Mode: C; c-basic-offset: 2; -*- */
/* GdkPixbuf library - test loaders
 *
 * Copyright (C) 2004 Matthias Clasen <mclasen@redhat.com>
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
 */

#include "config.h"
#include "gdk-pixbuf/gdk-pixbuf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static gboolean verbose = FALSE;

static void
load_image (gpointer  data, 
	    gpointer user_data)
{
  gchar *filename = data;
  FILE *file;
  int nbytes;
  guchar buf[1024];
  size_t bufsize = 1024;
  GdkPixbufLoader *loader;
  GError *error = NULL;
  GThread *self;

  self = g_thread_self ();
  loader = gdk_pixbuf_loader_new ();

  file = fopen (filename, "r");
  if (!file)
    {
      g_warning ("failed to open %s\n", filename);
      g_assert_not_reached ();
    }

  if (verbose) g_print ("%p start image %s\n", self, filename);
  while (!feof (file)) 
    {
      nbytes = fread (buf, 1, bufsize, file);
      if (!gdk_pixbuf_loader_write (loader, buf, nbytes, &error)) 
	{
	  g_warning ("Error writing %s to loader: %s", filename, error->message);
	  g_error_free (error);
          error = NULL;
	  break;
	}
      if (verbose) g_print ("%p read %d bytes\n", self, nbytes);

      g_thread_yield ();      
    }

  fclose (file);

  if (verbose) g_print ("%p finish image %s\n", self, filename);

  if (!gdk_pixbuf_loader_close (loader, &error)) 
    {
      g_warning ("Error closing loader for %s: %s", filename, error->message);
      g_error_free (error);
    }

  g_object_unref (loader);
}

int
main (int argc, char **argv)
{
  int i, start;
  GThreadPool *pool;
  GPtrArray *files = g_ptr_array_new_with_free_func ((GDestroyNotify)g_free);
  int l, iterations;

  g_test_init (&argc, &argv, NULL);

#if !GLIB_CHECK_VERSION (2, 35, 3)
  g_type_init ();
#endif

#if !GLIB_CHECK_VERSION (2, 32, 0)
  if (!g_thread_supported ())
    g_thread_init (NULL);
#endif

  if (g_getenv ("ITERATIONS"))
    iterations = atoi (g_getenv ("ITERATIONS"));
  else
    iterations = 1000;

  g_log_set_always_fatal (G_LOG_LEVEL_WARNING | G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL);

  start = 1;
  if (argc > 1 && strcmp (argv[1], "--verbose") == 0)
    {
      verbose = TRUE;
      start = 2;
    }

  pool = g_thread_pool_new (load_image, NULL, 20, FALSE, NULL);

  l = 0;

  for (i = start; i < argc; i++)
    g_ptr_array_add (files, argv[i]);

  if (files->len == 0)
    {
      const gchar *distdir = g_test_get_dir (G_TEST_DIST);
      g_ptr_array_add (files, g_build_filename (distdir, "test-images", "valid_jpeg_progressive_test", NULL));
      g_ptr_array_add (files, g_build_filename (distdir, "test-images", "valid_png_test", NULL));
    }

  g_assert_cmpint (files->len, >, 0);

  for (l = 0; l < iterations; l++)
    {
      for (i = 0; i < files->len; i++)
	{
	  g_thread_pool_push (pool, files->pdata[i], NULL);
	  if (verbose) g_print ("now %d items pending\n", g_thread_pool_unprocessed (pool));
	}
    }

  g_thread_pool_free (pool, FALSE, TRUE);

  return 0;
}
