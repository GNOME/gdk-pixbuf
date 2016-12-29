/* -*- Mode: C; c-basic-offset: 2; -*- */
/* GdkPixbuf library - test loaders
 *
 * Copyright (C) 2016 Red Hat, Inc.
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
 * Author: Andrey Tsyvarev
 *         Bastien Nocera
 */

#include <stdio.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

/* Auxiliary function, returns numeric representation of pixel.
 * For RGB format it is rrggbb(in hex), for RGBA - rrggbbaa. */
static guint32
get_pixel (GdkPixbuf *pixbuf, int x, int y)
{
    guchar *colors;
    guint32 pixel;

    colors = ((guchar*)gdk_pixbuf_get_pixels(pixbuf)
	      + gdk_pixbuf_get_n_channels(pixbuf) * x
	      + gdk_pixbuf_get_rowstride(pixbuf) * y);
    pixel = (colors[0] << 16) | (colors[1] << 8) | colors[2];

    if (gdk_pixbuf_get_n_channels (pixbuf) == 4)
        pixel = (pixel << 8) | colors[3];

    return pixel;
}
/* Verify whether all pixels outside the updated area
 * have the same values in pixbuf_old and pixbuf_new. */
static gboolean
pixbuf_not_changed_outside_area (GdkPixbuf *pixbuf_new,
				 GdkPixbuf *pixbuf_old,
				 int        x,
				 int        y,
				 int        width,
				 int        height)
{
    int pixbuf_width, pixbuf_height;
    int x_curr, y_curr;

    pixbuf_width = gdk_pixbuf_get_width (pixbuf_new);
    pixbuf_height = gdk_pixbuf_get_height (pixbuf_new);

    for (x_curr = 0; x_curr < pixbuf_width; x_curr++) {
        for (y_curr = 0; y_curr < pixbuf_height; y_curr++) {
            if ((x_curr >= x)
                && (x_curr < x + width)
                && (y_curr >= y)
                && (y_curr < y + height)) {
                continue; /* inside area - don't compare pixels. */
            }
            if (get_pixel (pixbuf_new, x_curr, y_curr) != get_pixel (pixbuf_old, x_curr, y_curr)) {
                printf("Pixel at (%d, %d) changed from %x to %x,\n",
                       x_curr, y_curr,
                       get_pixel (pixbuf_old, x_curr, y_curr),
                       get_pixel (pixbuf_new, x_curr, y_curr));
                printf("But it is outside of area with ");
                printf("x=%d, y=%d, width=%d, height=%d\n",
                       x, y, width, height);

#if 0
                gdk_pixbuf_save (pixbuf_old, "old.png", "png", NULL, NULL);
                gdk_pixbuf_save (pixbuf_new, "new.png", "png", NULL, NULL);
#endif

                g_assert_not_reached ();
            }
        }
    }

    return TRUE;
}

static void
callback_area_updated (GdkPixbufLoader *loader,
                       int              x,
                       int              y,
                       int              width,
                       int              height,
                       GdkPixbuf       *pixbuf_old)
{
    GdkPixbuf *pixbuf_new;

    pixbuf_new = gdk_pixbuf_loader_get_pixbuf (loader);

    pixbuf_not_changed_outside_area (pixbuf_new, pixbuf_old, x, y, width, height);
    /* update copy of pixbuf */
    gdk_pixbuf_copy_area (pixbuf_new, x, y, width, height, pixbuf_old, x, y);
}

/* free copy of pixbuf used in area-updated callback. */
static void
callback_closed (GdkPixbufLoader *loader,
                 GdkPixbuf       *pixbuf_copy)
{
    g_object_unref (pixbuf_copy);
}

/* prepare copy of pixbuf and connect other callbacks */
static void
callback_area_prepared (GdkPixbufLoader *loader)
{
    GdkPixbuf *pixbuf_copy;

    pixbuf_copy = gdk_pixbuf_copy (gdk_pixbuf_loader_get_pixbuf (loader));
    /* connect callbacks for another signals for not use pointer to pointer in them. */
    g_signal_connect (loader, "area-updated",
                      (GCallback) callback_area_updated, (gpointer) pixbuf_copy);
    g_signal_connect (loader, "closed",
                      (GCallback) callback_closed, (gpointer) pixbuf_copy);
}

/* Read count_bytes from the channel and write them to the loader.
 * Returns number of bytes written.
 * count_bytes = G_MAXSIZE means read as many bytes as possible. */
static gsize
loader_write_from_channel (GdkPixbufLoader *loader,
                           GIOChannel      *channel,
                           gsize            count_bytes)
{
    guchar* buffer;
    gsize bytes_read;
    GIOStatus read_status;

    if(count_bytes < G_MAXSIZE) {
        /* read no more than 'count_bytes' bytes */
        buffer = g_malloc(count_bytes);
        read_status = g_io_channel_read_chars (channel, (gchar*) buffer, count_bytes, &bytes_read, NULL);
    } else {
        /*read up to end */
        read_status = g_io_channel_read_to_end (channel, (gchar**) &buffer, &bytes_read, NULL);
    }

    if ((read_status != G_IO_STATUS_NORMAL) && (read_status != G_IO_STATUS_EOF))
        g_assert_not_reached ();

    if (!gdk_pixbuf_loader_write(loader, buffer, bytes_read, NULL))
        g_assert_not_reached ();

    g_free (buffer);
    return bytes_read;
}

static void
test_area_updated_ico (gconstpointer data)
{
    const char *filename;
    GIOChannel *channel;
    GdkPixbufLoader *loader;

    filename = g_test_get_filename (G_TEST_DIST, data, NULL);

    /* create channel */
    channel = g_io_channel_new_file(filename, "r", NULL);
    g_assert_nonnull (channel);
    g_io_channel_set_encoding (channel, NULL, NULL);
    /* create loader */
    loader = gdk_pixbuf_loader_new ();
    g_assert_nonnull (loader);

    g_signal_connect(loader, "area-prepared",
                    (GCallback) callback_area_prepared, NULL);
    /* callbacks for "area-updated" and "closed" signals will be connected
     * in callback_area_prepared() */

    /* read image byte by byte */
    while (loader_write_from_channel(loader, channel, 1) == 1);
    /* or read full image at once */
#if 0
    loader_write_from_channel(loader, channel, G_MAXSIZE);
#endif

    /* free resources */
    g_io_channel_unref (channel);

    gdk_pixbuf_loader_close (loader, NULL);
    g_object_unref (loader);
}

int main(int argc, char **argv)
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_data_func ("/pixbuf/area-updated/ico", (gconstpointer) "test-images/reftests/squares.ico", test_area_updated_ico);

  return g_test_run ();
}
