#[=======================================================================[.rst:
FindFFI
-------

Finds the Foreign Function Interface library, libffi.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``libffi::libffi``
  The libffi library

#]=======================================================================]

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_libffi QUIET libffi)
endif()

find_path(libffi_INCLUDE_DIR
  NAMES ffi.h
  HINTS ${PC_libffi_INCLUDE_DIRS}
  PATH_SUFFIXES ffi
)
find_library(libffi_LIBRARY
  NAMES ffi
  HINTS ${PC_libffi_LIBRARY_DIRS}
)

set(libffi_VERSION ${PC_libffi_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libffi
  REQUIRED_VARS
    libffi_LIBRARY
    libffi_INCLUDE_DIR
  VERSION_VAR libffi_VERSION
)

if(libffi_FOUND AND NOT TARGET libffi::libffi)
  add_library(libffi::libffi UNKNOWN IMPORTED)
  set_target_properties(libffi::libffi PROPERTIES
    IMPORTED_LOCATION "${libffi_LIBRARY}"
    INTERFACE_COMPILE_OPTIONS "${PC_libffi_CFLAGS_OTHER}"
    INTERFACE_INCLUDE_DIRECTORIES "${libffi_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(
  libffi_INCLUDE_DIR
  libffi_LIBRARY
)
