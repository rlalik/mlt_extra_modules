# - Try to find MLT
# Once done this will define
#  MLT_FOUND - System has LibXml2
#  MLT_INCLUDE_DIRS - The LibXml2 include directories
#  MLT_LIBRARIES - The libraries needed to use LibXml2
#  MLT_DEFINITIONS - Compiler switches required for using LibXml2

find_package(PkgConfig)
pkg_check_modules(PC_MLT QUIET mlt-framework)
set(MLT_DEFINITIONS ${PC_MLT_CFLAGS_OTHER})

find_path(MLT_INCLUDE_DIR framework/mlt.h
          HINTS ${PC_MLT_INCLUDEDIR} ${PC_MLT_INCLUDE_DIRS}
          PATH_SUFFIXES include/mlt )

find_library(MLT_LIBRARY NAMES mlt mlt++
             HINTS ${PC_MLT_LIBDIR} ${PC_MLT_LIBRARY_DIRS} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set MLT_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(MLT DEFAULT_MSG MLT_LIBRARY MLT_INCLUDE_DIR)

mark_as_advanced(MLT_INCLUDE_DIR MLT_LIBRARY )

set(MLT_LIBRARIES ${MLT_LIBRARY})
set(MLT_INCLUDE_DIRS ${MLT_INCLUDE_DIR})
