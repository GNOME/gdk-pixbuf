Contributing to GdkPixbuf
=========================

### Requirements

If you wish to contribute to GdkPixbuf you will need to install the
appropriate development tools for your operating system, including:

 - Python 3.x
 - Meson
 - Ninja
 - Gettext
 - a C99 compatible compiler

### Cloning and building

You can start by cloning the Git repository:

```sh
$ git clone https://gitlab.gnome.org/GNOME/gdk-pixbuf.git
$ cd gdk-pixbuf
```

Then you should build GdkPixbuf locally:

```sh
$ meson _build .
$ cd _build
$ ninja
```

### Development

Once you built GdkPixbuf, you should create a new branch in order
to work on your bug fix, or your feature, undisturbed:

```sh
$ git checkout -b my-amazing-feature
[ work work work ... ]
```

You should run the test suite locally, to verify you are not introducing a
regression; if you are fixing a bug, you should also add a test case to
verify that the fix works and to avoid future regressions; if you are
introducing a new feature, you should write a comprehensive test suite:

```sh
$ cd _build
$ meson test
```

When introducing a new feature or new API, you should document it using the
[gtk-doc](https://developer.gnome.org/gtk-doc-manual/stable/) format. You
can build the GdkPixbuf API reference locally by enabling the `docs`
configuration option and building the `gdk-pixbuf-doc` target:

```sh
$ cd _build
$ meson configure -Ddocs=true
$ ninja
$ ninja gdk-pixbuf-doc
```

### Contributing

Once you're done with your work, you should commit it, push it to a remote
repository, and open a Merge Request against the GdkPixbuf upstream
repository. Follow the [GitLab workflow page](https://wiki.gnome.org/GitLab/)
on the GNOME wiki for further instructions.

Once you opened a Merge Request, the GdkPixbuf maintainers will review your
contribution.

## Project layout

```
├── build-aux
├── contrib
│   └── gdk-pixbuf-xlib
├── docs
├── gdk-pixbuf
│   └── pixops
├── m4
├── po
├── tests
│   └── test-images
│       ├── fail
│       ├── randomly-modified
│       └── reftests
│           └── tga
└── thumbnailer
```

 - `build-aux`: Ancillary files, necessary to build GdkPixbuf
 - `contrib`: Additional functionality outside the main GdkPixbuf API
  - `gdk-pixbuf-xlib`: Xlib integration API; this library is considered
    deprecated, and should not be used in newly written code
 - `docs`: The GdkPixbuf API reference
 - `gdk-pixbuf`: The core GdkPixbuf source
  - `pixops`: Platform-specific code for pixel operations
 - `po`: Localization files
 - `tests`: Test suite
  - `test-images`: Reference images for the test suite
 - `thumbnailer`: Helper binary for generating thumbnails with GdkPixbuf

### Architecture

GdkPixbuf is divided into logical sections:

 - Core: the [GdkPixbuf][gdkpixbuf-api-core] object and its properties
 - Construction: [creating][gdkpixbuf-api-ctor] a new GdkPixbuf instance from a buffer
 - I/O: [Loading][gdkpixbuf-api-load] and [Saving][gdkpixbuf-api-save] image
   data in different formats
 - Image transformations: [Scaling and compositing][gdkpixbuf-api-ops] image
   data inside GdkPixbuf instances
 - The [GdkPixbuf loader][gdkpixbuf-api-loader] API, for incremental
   asynchronous loading of image data in a GdkPixbuf
 - The [loadable module interface][gdkpixbuf-api-module] for writing out of
   tree image loaders
 - The [animated image][gdkpixbuf-api-animation] API, for image formats
   that support animations

### Notes

GdkPixbuf is a library with a long history, and it has been incrementally
modified over years, so it may retain some older coding practices alongside
newer ones.

As it deals with loading image data into user processes, it's also important
to note that GdkPixbuf must always deal with potential security issues.

[gdkpixbuf-api-core]: https://developer.gnome.org/gdk-pixbuf/stable/gdk-pixbuf-The-GdkPixbuf-Structure.html
[gdkpixbuf-api-ctor]: https://developer.gnome.org/gdk-pixbuf/stable/gdk-pixbuf-Image-Data-in-Memory.html
[gdkpixbuf-api-load]: https://developer.gnome.org/gdk-pixbuf/stable/gdk-pixbuf-File-Loading.html
[gdkpixbuf-api-save]: https://developer.gnome.org/gdk-pixbuf/stable/gdk-pixbuf-File-saving.html
[gdkpixbuf-api-ops]: https://developer.gnome.org/gdk-pixbuf/stable/gdk-pixbuf-Scaling.html
[gdkpixbuf-api-loader]: https://developer.gnome.org/gdk-pixbuf/stable/GdkPixbufLoader.html
[gdkpixbuf-api-module]: https://developer.gnome.org/gdk-pixbuf/stable/gdk-pixbuf-Module-Interface.html
[gdkpixbuf-api-animation]: https://developer.gnome.org/gdk-pixbuf/stable/gdk-pixbuf-Animations.html
