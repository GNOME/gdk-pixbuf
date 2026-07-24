@echo on
:: vcvarsall.bat sets various env vars like PATH, INCLUDE, LIB, LIBPATH for the
:: specified build architecture
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
@echo on

@set RUST_HOST=x86_64-pc-windows-msvc

:: Needed for wget
@set MSYS2_BINDIR=c:\msys64\usr\bin

if not exist "_ccache" mkdir _ccache
set "CCACHE_BASEDIR=%CD%"
set "CCACHE_DIR=%CCACHE_BASEDIR%\_ccache"
:: Use depend_mode to reduce the cache miss overhead
:: https://ccache.dev/manual/latest.html#_the_depend_mode
set "CCACHE_DEPEND=1"
ccache --zero-stats
ccache --show-stats

pip3 install --upgrade --user meson==1.10.0rc2 || goto :error
set "CCACHE_DISABLE=true"
meson setup _build -Drelocatable=true -Dnative_windows_loaders=true -Dbuiltin_loaders=windows -Dman=false || goto :error
set "CCACHE_DISABLE="
ninja -C _build || goto :error
ccache --show-stats

goto :EOF
:error
exit /b 1
