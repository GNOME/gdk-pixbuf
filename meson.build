project('gdk-pixbuf', 'c',
        version: '2.36.10',
        license: 'LGPLv2.1+',
        default_options: [
          'buildtype=debugoptimized',
          'warning_level=1',
          'c_std=c99',
        ],
        meson_version: '>= 0.40.1')

add_project_arguments([ '-D_POSIX_C_SOURCE', '-D_DEFAULT_SOURCE' ], language: 'c')

cc = meson.get_compiler('c')
host_system = host_machine.system()

# Versioning
gdk_pixbuf_version = meson.project_version()
version_arr = gdk_pixbuf_version.split('.')
gdk_pixbuf_version_major = version_arr[0].to_int()
gdk_pixbuf_version_minor = version_arr[1].to_int()
gdk_pixbuf_version_micro = version_arr[2].to_int()

gdk_pixbuf_api_version = '2.0'
gdk_pixbuf_binary_version = '2.10.0'

gdk_pixbuf_api_name = '@0@-@1@'.format(meson.project_name(), gdk_pixbuf_api_version)

if gdk_pixbuf_version_minor.is_odd()
  gdk_pixbuf_interface_age = 0
else
  gdk_pixbuf_interface_age = gdk_pixbuf_version_micro
endif

gdk_pixbuf_binary_age = 100 * gdk_pixbuf_version_minor + gdk_pixbuf_version_micro

# maintaining compatibility with the previous libtool versioning
# current = binary - interface
# revision = interface
soversion = 0
current = gdk_pixbuf_binary_age - gdk_pixbuf_interface_age
revision = gdk_pixbuf_interface_age
libversion = '@0@.@1@.@2@'.format(soversion, current, revision)
age = gdk_pixbuf_binary_age - gdk_pixbuf_interface_age
current_minus_age = current - age

# Paths
gdk_pixbuf_prefix = get_option('prefix')
gdk_pixbuf_libdir = join_paths(gdk_pixbuf_prefix, get_option('libdir'))
gdk_pixbuf_bindir = join_paths(gdk_pixbuf_prefix, get_option('bindir'))
gdk_pixbuf_includedir = join_paths(gdk_pixbuf_prefix, get_option('includedir'))
gdk_pixbuf_datadir = join_paths(gdk_pixbuf_prefix, get_option('datadir'))
gdk_pixbuf_mandir = join_paths(gdk_pixbuf_prefix, get_option('mandir'))
gdk_pixbuf_localedir = join_paths(gdk_pixbuf_prefix, get_option('localedir'))
gdk_pixbuf_libexecdir = join_paths(gdk_pixbuf_prefix, get_option('libexecdir'))
gdk_pixbuf_loaderdir = join_paths(gdk_pixbuf_libdir, 'gdk-pixbuf-@0@/@1@/loaders'.format(gdk_pixbuf_api_version, gdk_pixbuf_binary_version))

# Dependencies
glib_req_version = '>= 2.38.0'
gio_dep = dependency('gio-2.0', version: glib_req_version)

# Configurations
gdk_pixbuf_conf = configuration_data()

check_headers = [
  'unistd.h',
  'sys/resource.h',
  'sys/time.h'
]

foreach h: check_headers
  if cc.has_header(h)
    gdk_pixbuf_conf.set('HAVE_' + h.underscorify().to_upper(), 1)
  endif
endforeach

# Look for the math library first, since we use it to test for round() and lrint()
mathlib_dep = cc.find_library('m', required: false)

# XXX: Remove the checks for round() and lrint() once GDK-Pixbuf is declared C99
if cc.has_function('round', dependencies: mathlib_dep)
  gdk_pixbuf_conf.set('HAVE_ROUND', 1)
endif

if cc.has_function('lrint', dependencies: mathlib_dep)
  gdk_pixbuf_conf.set('HAVE_LRINT', 1)
endif


# We use links() because sigsetjmp() is often a macro hidden behind other macros
gdk_pixbuf_conf.set('HAVE_SIGSETJMP',
                    cc.links('''#define _POSIX_SOURCE
                                #include <setjmp.h>
                                int main (void) {
                                  sigjmp_buf env;
                                  sigsetjmp (env, 0);
                                  return 0;
                                }''', name: 'sigsetjmp'))

# Common compiler and linker flags
common_cflags = []
common_ldflags = []

if cc.get_id() == 'msvc'
  # For Visual Studio, just force-include msvc_reommended_pragmas.h
  # so that we silence unwanted noise and track potential issues
  test_cflags = []
  add_project_arguments([ '-FImsvc_recommended_pragmas.h' ], language: 'c')
elif cc.get_id() == 'gcc' or cc.get_id() == 'clang'
  test_cflags = [
    '-Wpointer-arith',
    '-Wformat=2',
    '-Wstrict-prototypes',
    '-Wnested-externs',
    '-Wold-style-definition',
    '-Wdeclaration-after-statement',
    '-Wunused',
    '-Wcast-align',
    '-Wmissing-noreturn',
    '-Wmissing-format-attribute',
    '-Wlogical-op',
    '-fno-strict-aliasing',
    '-Wno-int-conversion',
    '-Wno-uninitialized',
    '-Wno-discarded-qualifiers',
    '-Werror=implicit',
    '-Werror=nonnull',
    '-Werror=init-self',
    '-Werror=main',
    '-Werror=missing-braces',
    '-Werror=sequence-point',
    '-Werror=return-type',
    '-Werror=trigraphs',
    '-Werror=array-bounds',
    '-Werror=write-strings',
    '-Werror=address',
    '-Werror=int-to-pointer-cast',
    '-Werror=pointer-to-int-cast',
    '-Werror=empty-body',
  ]

  # Ensure we have the correct bit packing on Windows
  if host_system == 'windows'
    test_cflags += '-mms-bitfields'
  endif
else
  test_cflags = []
endif

# Symbol visibility
if get_option('default_library') != 'static'
  if host_system == 'windows'
    gdk_pixbuf_conf.set('DLL_EXPORT', true)
    gdk_pixbuf_conf.set('_GDK_PIXBUF_EXTERN', '__declspec(dllexport) extern')
    if cc.get_id() != 'msvc'
      test_cflags += ['-fvisibility=hidden']
    endif
  else
    gdk_pixbuf_conf.set('_GDK_PIXBUF_EXTERN', '__attribute__((visibility("default"))) extern')
    test_cflags += ['-fvisibility=hidden']
  endif
endif

foreach cflag: test_cflags
  if cc.has_argument(cflag)
    common_cflags += cflag
  endif
endforeach

# Linker flags
if host_machine.system() == 'linux'
  ldflag_prog = 'int main (void) { return 0; }'
  foreach ldflag: [ '-Wl,-Bsymbolic', '-Wl,-z,relro', '-Wl,-z,now', ]
    if cc.links(ldflag_prog, args: ldflag, name: 'using @0@'.format(ldflag))
      common_ldflags += ldflag
    endif
  endforeach
endif

# Maintain compatibility with autotools on macOS
if host_machine.system() == 'darwin'
  common_ldflags += [ '-compatibility_version 1', '-current_version 1.0', ]
endif

# Dependencies
gobject_dep = dependency('gobject-2.0', version: glib_req_version)
gmodule_dep = dependency('gmodule-no-export-2.0')
gio_dep = dependency('gio-2.0')

# On non-Windows/macOS systems we always required shared-mime-info and GIO
if host_system != 'windows' and host_system != 'darwin'
  shared_mime_dep = dependency('shared-mime-info')
  gdk_pixbuf_conf.set('GDK_PIXBUF_USE_GIO_MIME', 1)
else
  shared_mime_dep = []
endif

gdk_pixbuf_deps = [ mathlib_dep, gobject_dep, gmodule_dep, gio_dep, shared_mime_dep ]

# Check if we can build shared modules
build_modules = gmodule_dep.get_pkgconfig_variable('gmodule_supported') == 'true'
gdk_pixbuf_conf.set10('USE_GMODULE', build_modules)

# Check which loaders should be built into gdk-pixbuf
builtin_loaders = get_option('builtin_loaders').split(',')

# If 'all' is specified for builtin_loaders, build all
# buildable loaders into gdk-pixbuf
builtin_all_loaders = false
if builtin_loaders == [ 'all' ]
  builtin_all_loaders = true
endif

# Loader dependencies
enabled_loaders = []
loaders_deps = []

if get_option('enable_png')
  # We have a vast selection of libpng versions to choose from
  foreach png: [ 'libpng16', 'libpng15', 'libpng14', 'libpng13', 'libpng12', 'libpng10' ]
    if not enabled_loaders.contains('png')
      png_dep = dependency(png, required: false)
      if png_dep.found()
        enabled_loaders += 'png'
        loaders_deps += png_dep
      endif
    endif
  endforeach

  if not enabled_loaders.contains('png')
    if cc.get_id() == 'msvc' and cc.has_header('png.h')
      # MSVC: First look for the DLL + import .lib build of libpng,
      # which is normally libpngxx.lib, when libpng's pkg-config can't
      # be found, which is quite normal on MSVC.
      foreach png: [ 'libpng16', 'libpng15', 'libpng14', 'libpng12', 'libpng13', 'libpng10' ]
        if not enabled_loaders.contains('png')
          png_dep = cc.find_library(png, required: false)
          if png_dep.found()
            enabled_loaders += 'png'
            loaders_deps += png_dep
          endif
        endif
      endforeach

      # If we still can't find libpng, try looking for the static libpng.lib,
      # which means we need to ensure we have the static zlib .lib as well
      if not enabled_loaders.contains('png')
        png_dep = cc.find_library('libpng', required: false)
        zlib_dep = cc.find_library('zlib', required: false)
        if png_dep.found() and zlib_dep.found()
          enabled_loaders += 'png'
          loaders_deps += [ png_dep, zlib_dep ]
        endif
      endif
    endif
  endif
endif

# On Windows, check whether we are building the native Windows loaders
# (it is an "all-or-nothing" option for BMP, EMF, GIF, ICO, JPEG, TIFF and WMF)
# Note that we currently do not use the native Windows loaders to handle PNG and
# JPEG2000 images
if host_system == 'windows'
  enable_native_windows_loaders = get_option('enable_native_windows_loaders')
else
  enable_native_windows_loaders = false
endif

if enable_native_windows_loaders
  loaders_deps += cc.find_library('gdiplus')
  loaders_deps += cc.find_library('ole32')
  enabled_loaders += 'gdiplus'
endif

# Don't check and build the jpeg loader if enable_native_windows_loaders is true
if get_option('enable_jpeg') and not enable_native_windows_loaders
  if cc.has_header('jpeglib.h')
    jpeg_dep = cc.find_library('jpeg', required: false)
    if cc.get_id() == 'msvc' and not jpeg_dep.found()
      # The IJG JPEG library builds the .lib file as libjpeg.lib in its MSVC build system,
      # so look for it as well when jpeg.lib cannot be found
      jpeg_dep = cc.find_library('libjpeg', required: false)
    endif
    if jpeg_dep.found() and cc.has_function('jpeg_destroy_decompress', dependencies: jpeg_dep)
      enabled_loaders += 'jpeg'
      loaders_deps += jpeg_dep

      gdk_pixbuf_conf.set('HAVE_PROGRESSIVE_JPEG',
                          cc.has_function('jpeg_simple_progression', dependencies: jpeg_dep))
    endif
  endif
endif

# Don't check and build the tiff loader if enable_native_windows_loaders is true
if get_option('enable_tiff') and not enable_native_windows_loaders
  tiff_dep = dependency('libtiff-4', required: false)
  if not tiff_dep.found()
    # Fallback when no pkg-config file is found for libtiff on MSVC, which is quite normal
    if cc.get_id() == 'msvc' and cc.has_header('tiff.h')
      # First look for the DLL builds of libtiff, then the static builds
      tiff_dep = cc.find_library('libtiff_i', required: false)

      if not tiff_dep.found()
        # For the static lib, zlib and libjpeg .lib's have been looked for first, and
        # they are optional for libtiff
        tiff_dep = cc.find_library('libtiff', required: false)
      endif
    endif
  endif
  if tiff_dep.found()
    enabled_loaders += 'tiff'
    loaders_deps += tiff_dep
  endif
endif

jasper_extra_cflags = []

if get_option('enable_jasper')
  has_jasper_header = false

  if cc.get_id() == 'msvc'
    # We must define JAS_WIN_MSVC_BUILD before including jasper/jasper.h on MSVC
    jasper_msvc_macro = 'JAS_WIN_MSVC_BUILD'
    has_jasper_header = cc.has_header('jasper/jasper.h', prefix: '#define @0@'.format(jasper_msvc_macro))
    jasper_extra_cflags = [ '-D@0@'.format(jasper_msvc_macro) ]
  else
    has_jasper_header = cc.has_header('jasper/jasper.h')
  endif

  if has_jasper_header
    jasper_dep = cc.find_library('jasper', required: false)
    if not jasper_dep.found()
      if cc.get_id() == 'msvc'
        # on MSVC, we need to look for libjasper.lib if jasper.lib is not found,
        # which corresponds to what libjasper's MSVC projects produce
        jasper_dep = cc.find_library('libjasper', required: false)
      endif
    endif
    if jasper_dep.found() and cc.has_function('jas_init', dependencies: jasper_dep)
      enabled_loaders += 'jasper'
      loaders_deps += jasper_dep
    endif
  endif
endif

# Determine whether we enable application bundle relocation support, and we use
# this always on Windows
if host_system == 'windows'
  relocatable = true
else
  relocatable = get_option('enable_relocatable')
endif

if relocatable
  add_project_arguments([ '-DGDK_PIXBUF_RELOCATABLE' ], language: 'c')
endif

gdk_pixbuf_conf.set_quoted('GETTEXT_PACKAGE', meson.project_name())

configure_file(output: 'config.h', configuration: gdk_pixbuf_conf)

# Compat variables for pkgconfig
pkgconf = configuration_data()
pkgconf.set('prefix', gdk_pixbuf_prefix)
pkgconf.set('exec_prefix', gdk_pixbuf_prefix)
pkgconf.set('libdir', gdk_pixbuf_libdir)
pkgconf.set('includedir', gdk_pixbuf_includedir)
pkgconf.set('GDK_PIXBUF_API_VERSION', gdk_pixbuf_api_version)
pkgconf.set('GDK_PIXBUF_BINARY_VERSION', gdk_pixbuf_binary_version)
pkgconf.set('GDK_PIXBUF_EXTRA_CFLAGS', '')
pkgconf.set('GDK_PIXBUF_EXTRA_LIBS', '')
pkgconf.set('PNG_DEP_CFLAGS_PACKAGES', '')
pkgconf.set('VERSION', meson.project_version())

configure_file(input: 'gdk-pixbuf-2.0.pc.in',
               output: 'gdk-pixbuf-2.0.pc',
               configuration: pkgconf,
               install: true,
               install_dir: join_paths(gdk_pixbuf_libdir, 'pkgconfig'))

root_inc = include_directories('.')

# Auxiliary scripts
gen_resources = find_program('build-aux/gen-resources.py')
gen_installed_test = find_program('build-aux/gen-installed-test.py')
gen_thumbnailer = find_program('build-aux/gen-thumbnailer.py')

gnome = import('gnome')

subdir('gdk-pixbuf')

# X11
x11_dep = dependency('x11', required: false)
if x11_dep.found()
  subdir('contrib/gdk-pixbuf-xlib')
endif

# i18n
subdir('po')

subdir('tests')
subdir('thumbnailer')

# Documentation
subdir('docs/reference/gdk-pixbuf')
