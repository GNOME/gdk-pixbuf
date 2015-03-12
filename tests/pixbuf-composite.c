#include <gdk-pixbuf.h>
#include "test-common.h"

static void
test_composite (void)
{
  GdkPixbuf *red, *green, *out, *ref, *sub;

  red = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, 24, 24);
  gdk_pixbuf_fill (red, 0xff000000);

  green = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, 12, 12);
  gdk_pixbuf_fill (green, 0x00ff0000);

  out = gdk_pixbuf_copy (red);

  gdk_pixbuf_composite (green, out,
                        0, 0, 12, 12,
                        0, 0, 1.0, 1.0,
                        GDK_INTERP_NEAREST,
                        255);

  ref = gdk_pixbuf_copy (red);
  sub = gdk_pixbuf_new_subpixbuf (ref, 0, 0, 12, 12);

  gdk_pixbuf_fill (sub, 0x00ff0000);

  g_assert (pixdata_equal (out, ref, NULL));

  g_object_unref (red);
  g_object_unref (green);
  g_object_unref (out);
  g_object_unref (ref);
  g_object_unref (sub);
}

int
main (int argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/pixbuf/composite", test_composite);

  return g_test_run ();
}
