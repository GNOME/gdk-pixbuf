# GDK-Pixbuf - An image loading library

GDK-Pixbuf loads, manipulates, and saves image data from files, memory areas,
and streams.

GDK-Pixbuf supports multiple file and image formats, and can be extended to
support third party loaders.

Once an image is loaded, GDK-Pixbuf exposes it as a memory area in order to
perform pixel-based operations.

## Building GDK-Pixbuf

Once you cloned the Git repository, using:

```sh
$ git clone https://git.gnome.org/browse/gdk-pixbuf
```

You can build GDK-Pixbuf using [Meson][meson-web] and [Ninja][ninja-web]:

```sh
$ meson _build .
$ ninja -C _build
```

You can run the test suite using `mesontest`:

```sh
$ mesontest -C _build
```

And, finally, you can install GDK-Pixbuf using Ninja:

```
$ sudo ninja -C _build install
```

## Dependencies

GDK-Pixbuf depends on [GLib][glib-git] for its basic types. Additionally,
there are dependencies for image loading:

  - libpng
  - libtiff
  - libjpeg
  - libjasper

## Licensing

GDK-Pixbuf is released under the terms of the GNU Library General Public
License, version 2 or, at your option, later versions. See the `COPYING`
file in the project's root directory for further information.

[meson-web]: http://mesonbuild.com
[ninja-web]: https://ninja-build.org
[glib-git]: https://git.gnome.org/browse/glib
