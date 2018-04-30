#!/bin/sh

libdir="$1"
binary_version="$2"

if [ -z "$DESTDIR" ]; then
        mkdir -p "$libdir/gdk-pixbuf-2.0/$binary_version"
        gdk-pixbuf-query-loaders > "$libdir/gdk-pixbuf-2.0/$binary_version/loaders.cache"
else
        echo "***"
        echo "*** Warning: loaders.cache not built"
        echo "***"
        echo "*** You should generate this file manually on the host system"
        echo "*** using:"
        echo "***   gdk-pixbuf-query-loaders > $libdir/gdk-pixbuf-2.0/$binary_version/loaders.cache"
        echo "***"
fi
