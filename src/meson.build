# generate openslide-tables.c
make_tables = executable(
  'make-tables', 'make-tables.c',
  dependencies : [libm_native_dep],
  native : true,
)
openslide_tables_c = custom_target(
  'make openslide-tables',
  output : 'openslide-tables.c',
  command : [make_tables, '@OUTPUT@'],
)

# Windows resources
openslide_dll_manifest = configure_file(
  input : 'openslide-dll.manifest.in',
  output : 'openslide-dll.manifest',
  configuration : versions,
)
openslide_dll_rc = configure_file(
  input : 'openslide-dll.rc.in',
  output : 'openslide-dll.rc',
  configuration : versions,
)
if host_machine.system() == 'windows'
  openslide_dll_o = import('windows').compile_resources(
    openslide_dll_rc,
    depend_files : [openslide_dll_manifest],
  )
else
  openslide_dll_o = files()
endif

# Public headers
openslide_headers = files(
  'openslide.h',
  'openslide-features.h',
)
include_subdir = 'openslide'
install_headers(
  openslide_headers,
  subdir : include_subdir,
)

# Library
openslide_sources = [
  'openslide.c',
  'openslide-cache.c',
  openslide_dll_o,
  'openslide-decode-jp2k.c',
  'openslide-decode-jpeg.c',
  'openslide-decode-tiff.c',
  'openslide-decode-tifflike.c',
  'openslide-error.c',
  'openslide-file.c',
  'openslide-grid.c',
  'openslide-hash.c',
  'openslide-jdatasrc.c',
  openslide_tables_c,
  'openslide-util.c',
  'openslide-vendor-aperio.c',
]
libopenslide_link_args += [
  '-Wl,--gc-sections',
]
libopenslide = library('openslide',
  openslide_sources,
  version : soversion,
  c_args : ['-D_OPENSLIDE_BUILDING_DLL', '-DG_LOG_DOMAIN="OpenSlide"'],
  link_args : libopenslide_link_args,
  link_language : 'cpp',
  gnu_symbol_visibility : visibility,
  include_directories : config_h_include,
  dependencies : [
    glib_dep,
    cairo_dep,
    tiff_dep,
    openjpeg_dep,
    jpeg_dep,
    zlib_dep,
    libm_dep,
    s3t_dep,
    awssdk_dep,
  ],
  install : true,
)
openslide_dep = declare_dependency(
  include_directories : include_directories('.'),
  link_with : libopenslide,
)
import('pkgconfig').generate(
  libopenslide,
  description : 'A library for reading whole slide images.',
  subdirs : include_subdir,
  url : 'https://openslide.org',
)
if meson.version().version_compare('>=0.54')
  meson.override_dependency('openslide', openslide_dep)
endif
