# - Try to find Kdenlive
# Once done this will define
#  Kdenlive_FOUND - System has LibXml2
#  Kdenlive_INCLUDE_DIRS - The LibXml2 include directories
#  Kdenlive_LIBRARIES - The libraries needed to use LibXml2
#  Kdenlive_DEFINITIONS - Compiler switches required for using LibXml2

find_path(Kdenlive_DATA kdenlivedefaultlayouts.rc
          HINTS /usr/share
          PATH_SUFFIXES kdenlive)
message(STATUS "DATA_DIR ${Kdenlive_DATA_DIR}")

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set Kdenlive_FOUND to TRUE
# if all listed variables are TRUE
#find_package_handle_standard_args(Kdenlive DEFAULT_MSG Kdenlive_LIBRARY Kdenlive_INCLUDE_DIR)

mark_as_advanced(Kdenlive_DATA_DIR)

set(Kdenlive_DATA_DIR ${Kdenlive_DATA})
