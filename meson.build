project('gdk-pixbuf', 'c',
        version: '2.36.8',
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
  'unistd.h'
]

foreach h: check_headers
  if cc.has_header(h)
    gdk_pixbuf_conf.set('HAVE_' + h.underscorify().to_upper(), 1)
  endif
endforeach

# We use links() because sigsetjmp() is often a macro hidden behind other macros
gdk_pixbuf_conf.set('HAVE_SIGSETJMP',
                    cc.links('''#define _POSIX_SOURCE
                                #include <setjmp.h>
                                int main (void) {
                                  sigjmp_buf env;
                                  sigsetjmp (env, 0);
                                  return 0;
                                }''', name: 'sigsetjmp'))

# XXX: Remove once we declare gdk-pixbuf C99-only
if cc.get_id() != 'msvc'
  gdk_pixbuf_conf.set('HAVE_ROUND', 1)
  gdk_pixbuf_conf.set('HAVE_LRINT', 1)
endif

# Common compiler and linker flags
common_cflags = []
common_ldflags = []

if cc.get_id() == 'msvc'
  # Compiler options taken from msvc_recommended_pragmas.h
  # in GLib, based on _Win32_Programming_ by Rector and Newcomer
  test_cflags = [
    '-we4002', # too many actual parameters for macro
    '-we4003', # not enough actual parameters for macro
    '-w14010', # single-line comment contains line-continuation character
    '-we4013', # 'function' undefined; assuming extern returning int
    '-w14016', # no function return type; using int as default
    '-we4020', # too many actual parameters
    '-we4021', # too few actual parameters
    '-we4027', # function declared without formal parameter list
    '-we4029', # declared formal parameter list different from definition
    '-we4033', # 'function' must return a value
    '-we4035', # 'function' : no return value
    '-we4045', # array bounds overflow
    '-we4047', # different levels of indirection
    '-we4049', # terminating line number emission
    '-we4053', # an expression of type void was used as an operand
    '-we4071', # no function prototype given
    '-we4819', # the file contains a character that cannot be represented in the current code page
  ]
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
mathlib_dep = cc.find_library('m', required: false)
gobject_dep = dependency('gobject-2.0', version: glib_req_version)
gmodule_dep = dependency('gmodule-no-export-2.0')
gio_dep = dependency('gio-2.0')

# On non-Windows systems we always required shared-mime-info and GIO
if host_machine.system() != 'windows'
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

# Loader dependencies
enabled_loaders = []
loaders_deps = []

if get_option('enable_png')
  # We have a vast selection of libpng versions to choose from
  foreach png: [ 'libpng16', 'libpng15', 'libpng14', 'libpng12', 'libpng13', 'libpng10' ]
    if not enabled_loaders.contains('png')
      png_dep = dependency(png, required: false)
      if png_dep.found()
        enabled_loaders += 'png'
        loaders_deps += png_dep
      endif
    endif
  endforeach
endif

if get_option('enable_tiff')
  tiff_dep = dependency('libtiff-4', required: false)
  if tiff_dep.found()
    enabled_loaders += 'tiff'
    loaders_deps += tiff_dep
  endif
endif

if get_option('enable_jpeg')
  if cc.has_header('jpeglib.h')
    jpeg_dep = cc.find_library('jpeg', required: false)
    if jpeg_dep.found() and cc.has_function('jpeg_destroy_decompress', dependencies: jpeg_dep)
      enabled_loaders += 'jpeg'
      loaders_deps += jpeg_dep

      gdk_pixbuf_conf.set('HAVE_PROGRESSIVE_JPEG',
                          cc.has_function('jpeg_simple_progression', dependencies: jpeg_dep))
    endif
  endif
endif

if get_option('enable_jasper')
  if cc.has_header('jasper/jasper.h')
    jasper_dep = cc.find_library('jasper', required: false)
    if jasper_dep.found() and cc.has_function('jas_init', dependencies: jasper_dep)
      enabled_loaders += 'jasper'
      loaders_deps += jasper_dep
    endif
  endif
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
