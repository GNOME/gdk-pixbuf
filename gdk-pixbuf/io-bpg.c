/* BPG loader
 *
 * Copyright (c) 2016 Valérian Rousset <valerian.rousset@gmail.com>
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

#include "config.h"
#include <stdlib.h>
#include <stdio.h>

#include "gdk-pixbuf-private.h"

#include <stdint.h>
#include <libbpg.h>

G_MODULE_EXPORT void fill_vtable(GdkPixbufModule * module);
G_MODULE_EXPORT void fill_info(GdkPixbufFormat * info);

static GdkPixbuf *get_pixbuf(BPGDecoderContext * img, GError ** error)
{
	BPGImageInfo img_info;

	bpg_decoder_get_info(img, &img_info);
	size_t width = img_info.width;
	size_t height = img_info.height;

	BPGDecoderOutputFormat out_format;
	size_t real_bit_depth;
	switch (img_info.bit_depth) {
	case 8:
		if (img_info.has_alpha) {
			out_format = BPG_OUTPUT_FORMAT_RGBA32;
			real_bit_depth = 32;
		} else {
			out_format = BPG_OUTPUT_FORMAT_RGB24;
			real_bit_depth = 24;
		}
		break;
	default:
		g_set_error(error,
			    GDK_PIXBUF_ERROR,
			    GDK_PIXBUF_ERROR_FAILED, _("Invalid bit_depth"));

		return NULL;
	}

	size_t line_size = real_bit_depth / 8;

	GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB,
					   img_info.has_alpha,
					   8,
					   width,
					   height);
	guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);

	bpg_decoder_start(img, out_format);
	for (size_t y = 0; y < height; ++y) {
		guchar *pixel_dest = pixels + (width * y * line_size);
		bpg_decoder_get_line(img, pixel_dest);
	}

	return pixbuf;
}

static GdkPixbuf *bpg_image_load(FILE * file, GError ** error)
{
	fseek(file, 0, SEEK_END);
	size_t file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	uint8_t *file_content = calloc(file_size, sizeof(*file_content));
	if (fread(file_content, 1, file_size, file) != file_size) {
		g_set_error(error,
			    GDK_PIXBUF_ERROR,
			    GDK_PIXBUF_ERROR_FAILED, _("Filed changed size"));
		free(file_content);
		return NULL;
	}

	BPGDecoderContext *img = bpg_decoder_open();
	if (bpg_decoder_decode(img, file_content, file_size) < 0) {
		g_set_error(error,
			    GDK_PIXBUF_ERROR,
			    GDK_PIXBUF_ERROR_FAILED,
			    _("Unable to decode image"));
		bpg_decoder_close(img);
		free(file_content);
		return NULL;
	}
	free(file_content);

	GdkPixbuf *pixbuf = get_pixbuf(img, error);

	bpg_decoder_close(img);

	return pixbuf;
}

#ifndef INCLUDE_bpg
#define MODULE_ENTRY(function) G_MODULE_EXPORT void function
#else
#define MODULE_ENTRY(function) void _gdk_pixbuf__bpg_ ## function
#endif

MODULE_ENTRY(fill_vtable) (GdkPixbufModule * module) {
	module->load = bpg_image_load;
}

MODULE_ENTRY(fill_info) (GdkPixbufFormat * info) {
	static const GdkPixbufModulePattern signature[] = {
		{"\x42\x50\x47\xfb", NULL, 100},
		{NULL, NULL, 0}
	};
	static const gchar *mime_types[] = {
		"application/octet-stream",
		NULL
	};
	static const gchar *extensions[] = {
		"bpg",
		NULL
	};

	info->name = "bpg";
	info->signature = (GdkPixbufModulePattern *) signature;
	info->description = NC_("image format", "Better Portable Graphics");
	info->mime_types = (gchar **) mime_types;
	info->extensions = (gchar **) extensions;
	info->flags = GDK_PIXBUF_FORMAT_THREADSAFE;
	info->license = "LGPL";
	info->disabled = FALSE;
}
