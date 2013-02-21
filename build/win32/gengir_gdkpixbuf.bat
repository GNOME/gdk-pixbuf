@echo off

setlocal EnableDelayedExpansion

rem Needed environmental variables:
rem PLAT: Windows platform-Win32 (i.e. x86) or x64 (i.e. x86-64)
rem CONF: Configuration Type, Release or Debug
rem VSVER: Visual C++ version used [9, 10 or 11]
rem BASEDIR: Where the dependent libraries/headers are located
rem PKG_CONFIG_PATH: Where the GLib and its dependent pkg-config .pc files can be found
rem MINGWDIR: Installation path of MINGW GCC, so gcc.exe can be found in %MINGWDIR%\bin.

rem Note that the Python executable/installation and all the runtime dependencies of the
rem library/libraries need to be in your PATH or %BASEBIN%\bin.

rem Check the environemental variables...
if /i "%PLAT%" == "Win32" goto PLAT_OK
if /i "%PLAT%" == "x64" goto PLAT_OK
if /i "%PLAT%" == "x86" (
   set PLAT=Win32
   goto PLAT_OK
)
if /i "%PLAT%" == "x86-64" (
   set PLAT=x64
   goto PLAT_OK
)
goto ERR_PLAT

:PLAT_OK
if "%VSVER%" == "9" goto VSVER_OK
if "%VSVER%" == "10" goto VSVER_OK
if "%VSVER%" == "11" goto VSVER_OK
goto ERR_VSVER
:VSVER_OK
if /i "%CONF%" == "Release" goto CONF_OK
if /i "%CONF%" == "Debug" goto CONF_OK
goto ERR_CONF
:CONF_OK
if "%BASEDIR%" == "" goto ERR_BASEDIR
if not exist %BASEDIR% goto ERR_BASEDIR

if "%PKG_CONFIG_PATH%" == "" goto ERR_PKGCONFIG
if not exist %PKG_CONFIG_PATH%\gobject-2.0.pc goto ERR_PKGCONFIG

if "%MINGWDIR%" == "" goto ERR_MINGWDIR
if not exist %MINGWDIR%\bin\gcc.exe goto ERR_MINGWDIR

set CC=cl
set BINDIR=%CD%\vs%VSVER%\%CONF%\%PLAT%\bin
set INCLUDE=%BASEDIR%\include\glib-2.0;%BASEDIR%\lib\glib-2.0\include;%INCLUDE%
set LIB=%BINDIR%;%BASEDIR%\lib;%LIB%
set PATH=%BINDIR%;%BASEDIR%\bin;%PATH%;%MINGWDIR%\bin
set PYTHONPATH=%BASEDIR%\lib\gobject-introspection;%BINDIR%

echo Setup filelist for generating GdkPixbuf .gir's...

call python gen-file-list-gdkpixbuf.py

echo Setup .bat for generating GdkPixbuf .gir's...

rem ===============================================================================
rem Begin setup of gdkpixbuf_gir.bat to create GdkPixbuf-2.0.gir
rem (The ^^ is necessary to span the command to multiple lines on Windows cmd.exe!)
rem ===============================================================================

echo echo Generating GdkPixbuf-2.0.gir...> gdkpixbuf_gir.bat
echo @echo off>> gdkpixbuf_gir.bat
echo.>> gdkpixbuf_gir.bat
echo copy /b %BINDIR%\gdk_pixbuf-2.0.lib %BINDIR%\gdkpixbuf-2.0.lib>> gdkpixbuf_gir.bat
echo.>> gdkpixbuf_gir.bat
rem =====================================================================
rem Setup the command line flags to g-ir-scanner for GdkPixbuf-2.0.gir...
rem =====================================================================
echo python %BASEDIR%\bin\g-ir-scanner --verbose -I..\.. -I..\..\gdk-pixbuf ^^>> gdkpixbuf_gir.bat
echo -I%BASEDIR%\include\glib-2.0 -I%BASEDIR%\lib\glib-2.0\include ^^>> gdkpixbuf_gir.bat
echo --namespace=GdkPixbuf --nsversion=2.0 ^^>> gdkpixbuf_gir.bat
echo --include=GModule-2.0 --include=Gio-2.0 ^^>> gdkpixbuf_gir.bat
echo --no-libtool --library=gdk_pixbuf-2-vs%VSVER% ^^>> gdkpixbuf_gir.bat
echo --reparse-validate --add-include-path=%BASEDIR%\share\gir-1.0 --add-include-path=. ^^>> gdkpixbuf_gir.bat
echo --pkg-export gdk-pixbuf-2.0 --warn-all --strip-prefix=Gdk --c-include="gdk-pixbuf/gdk-pixbuf.h" ^^>> gdkpixbuf_gir.bat
echo -I..\.. -DGDK_PIXBUF_COMPILATION ^^>> gdkpixbuf_gir.bat
echo --filelist=gdkpixbuf_list ^^>> gdkpixbuf_gir.bat
echo -o GdkPixbuf-2.0.gir>> gdkpixbuf_gir.bat
echo.>> gdkpixbuf_gir.bat

echo del %BINDIR%\gdkpixbuf-2.0.lib>> gdkpixbuf_gir.bat
echo.>> gdkpixbuf_gir.bat
echo Completed setup of .bat for generating GdkPixbuf-2.0.gir.
echo.>> gdkpixbuf_gir.bat

rem =============================================================
rem Finish setup of gdkpixbuf_gir.bat to create GdkPixbuf-2.0.gir
rem =============================================================

rem =======================
rem Now generate the .gir's
rem =======================
CALL gdkpixbuf_gir.bat

rem Clean up the .bat and the filelist for generating the .gir files...
del gdkpixbuf_gir.bat
del gdkpixbuf_list

rem Now compile the generated .gir files
%BASEDIR%\bin\g-ir-compiler --includedir=. --debug --verbose GdkPixbuf-2.0.gir -o GdkPixbuf-2.0.typelib
rem Copy the generated .girs and .typelibs to their appropriate places

mkdir ..\..\build\win32\vs%VSVER%\%CONF%\%PLAT%\share\gir-1.0
move /y *.gir %BASEDIR%\share\gir-1.0\

mkdir ..\..\build\win32\vs%VSVER%\%CONF%\%PLAT%\lib\girepository-1.0
move /y *.typelib %BASEDIR%\lib\girepository-1.0\

goto DONE

:ERR_PLAT
echo You need to specify a valid Platform [set PLAT=Win32 or PLAT=x64]
goto DONE
:ERR_VSVER
echo You need to specify your Visual Studio version [set VSVER=9 or VSVER=10 or VSVER=11]
goto DONE
:ERR_CONF
echo You need to specify a valid Configuration [set CONF=Release or CONF=Debug]
goto DONE
:ERR_BASEDIR
echo You need to specify a valid BASEDIR.
goto DONE
:ERR_PKGCONFIG
echo You need to specify a valid PKG_CONFIG_PATH
goto DONE
:ERR_MINGWDIR
echo You need to specify a valid MINGWDIR, where a valid gcc installation can be found.
goto DONE
:DONE

