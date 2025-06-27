#pragma once

#include "gdk-pixbuf-io.h"

void glycin_fill_vtable (GdkPixbufModule *module);

gboolean glycin_image_save (const char         *mimetype,
                            FILE               *f,
                            GdkPixbufSaveFunc   save_func,
                            gpointer            user_data,
                            GdkPixbuf          *pixbuf,
                            char              **keys,
                            char              **values,
                            GBytes             *icc_profile,
                            int                 quality,
                            int                 compression,
                            GError            **error);
