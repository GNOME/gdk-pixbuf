#include <gdk-pixbuf/gdk-pixbuf.h>
#include <stdlib.h>

static void
test_animation (void)
{
  GError *error = NULL;
  GdkPixbufAnimation* result = NULL;

  result = gdk_pixbuf_animation_new_from_file (g_test_get_filename (G_TEST_DIST, "test-animation.gif", NULL), &error);
  g_assert_no_error (error);
  g_assert (result != NULL);

  g_object_unref (result);
}

int
main (int argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/animation/load", test_animation);

  return g_test_run ();
}
