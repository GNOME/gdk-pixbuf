GdkPixbuf: Image loading library
================================

GdkPixbuf is a library that loads image data in various formats and stores
it as linear buffers in memory. The buffers can then be scaled, composited,
modified, saved, or rendered.

GdkPixbuf can load image data encoded in different formats, such as:

 - PNG
 - JPEG
 - TIFF
 - TGA
 - GIF

Additionally, you can write a GdkPixbuf loader module and install it into
a well-known location, in order to load a file format.

GdkPixbuf is used by the [GTK](https://www.gtk.org) toolkit for loading
graphical assets.

## Building GdkPixbuf

### Requirements

In order to build GdkPixbuf you will need to have installed:

 - Meson
 - A C99-compliant compiler and toolchain
 - GLib's development files

Depending on the image formats you want to support you will also need:

 - libpng's development files
 - libjpeg's development files
 - libtiff's development files
 - libjasper's development files

Additionally, you may need:

 - shared-mime-info
 - GObject Introspection
 - GTK-Doc
 - Xlib's development files
 - mediaLib's development files

### Building and installing

You should use Meson to configure GdkPixbuf's build, and depending on the
platform you will be able to use Ninja, Visual Studio, or XCode to build
the project; typically, on most platforms, you should be able to use the
following commands to build and install GdkPixbuf in the default prefix:

```sh
$ meson _build .
$ cd _build
$ ninja
# ninja install
```

You can use the `--prefix` argument to control the installation prefix.

You can also use `meson configure` from within the build directory to
check the current build configuration, and change its options.

## License

GdkPixbuf is released under the terms of the GNU Library General Public
License version 2, or, at your option, any later version. See the
[COPYING](./COPYING) file for further details.
