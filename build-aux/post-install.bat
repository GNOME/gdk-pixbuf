@echo off

set bindir=%1
set libdir=%2
set binary_version=%3

set libdir_windows=%libdir:/=\%

if not "%DESTDIR%" == "" goto warn_msg
if not exist %libdir_windows%\gdk-pixbuf-2.0\%binary_version%\ mkdir %libdir_windows%\gdk-pixbuf-2.0\%binary_version%
%bindir%\gdk-pixbuf-query-loaders > %libdir_windows%\gdk-pixbuf-2.0\%binary_version%\loaders.cache

goto end

:warn_msg
echo ***
echo *** Warning: loaders.cache not built
echo ***
echo *** You should generate this file manually on the host system
echo *** using:
echo ***   gdk-pixbuf-query-loaders ^> %libdir_windows%\gdk-pixbuf-2.0\%binary_version%\loaders.cache
echo ***

:end
