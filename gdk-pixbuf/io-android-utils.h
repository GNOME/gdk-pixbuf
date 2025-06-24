/* GdkPixbuf library - Android image loader
 *
 * Copyright (C) 2025  Florian Leander Singer <sp1rit@disroot.org>
 *
 * Authors: Florian Leander Singer <sp1rit@disroot.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __IO_ANDROID_UTILS_H__
#define __IO_ANDROID_UTILS_H__

#include <android/bitmap.h>

#include "gdk-pixbuf-io.h"
#include "gdk-pixbuf-animation.h"

void gdk_pixbuf__android_fill_vtable (GdkPixbufModule *module);

gboolean gdk_pixbuf__android_save_image (GdkPixbufSaveFunc save_func, gpointer user_data, GdkPixbuf *pixbuf, gint32 format, GError **error);

#endif // __IO_ANDROID_UTILS_H__
