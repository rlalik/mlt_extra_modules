cmake_minimum_required(VERSION 3.10)
project(MltExtraPackages
    VERSION 0.3.3
    LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

find_package(PkgConfig REQUIRED)
find_package(MLT REQUIRED)
find_package(Qt5 COMPONENTS Core Xml REQUIRED)

option(KDENLIVE "Install Kdenlive effects file" ON)

include(GNUInstallDirs)
option(GPL "Enable GPLv2 modules" ON)
option(GPL3 "Enable GPLv3 modules" ON)
if(WIN32)
  option(NODEPLOY "Keep bin/ lib/ layout on Windows" ON)
endif()
if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64)|(AMD64)")
    set(X86_64 ON)
    if(NOT MSVC)
        add_definitions(-DUSE_MMX -DUSE_SSE -DUSE_SSE2 -DARCH_X86_64)
    endif()
endif()
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-ffast-math)
elseif(MSVC)
    add_compile_options(/fp:fast)
endif()
find_package(Threads REQUIRED)
find_package(FFTW QUIET)
if(NOT FFTW_FOUND)
    pkg_check_modules(FFTW fftw3)
endif()
if(NOT FFTW_FOUND)
    pkg_check_modules(FFTW fftw)
endif()
pkg_check_modules(xml IMPORTED_TARGET libxml-2.0)

include_directories(${MLT_INCLUDE_DIRS})
set(modules
    src/modules/rl
)
foreach(module ${modules})
    add_subdirectory(${module})
endforeach()

if (KDENLIVE)
    find_package(Kdenlive REQUIRED)
    install(FILES 
        data/typewriter.xml
    DESTINATION ${Kdenlive_DATA_DIR}/effects)
endif()

# ===============================
# Cmake's Target and Config

set(CMAKE_INSTALL_CMAKEDIR ${CMAKE_INSTALL_LIBDIR}/cmake/${CMAKE_PROJECT_NAME})

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE ${CMAKE_PROJECT_NAME})

include(CMakePackageConfigHelpers)
configure_package_config_file(Config.cmake.in ${CMAKE_PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_CMAKEDIR}
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR CMAKE_INSTALL_BINDIR)

write_basic_package_version_file(${CMAKE_PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

install(FILES 
    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_CMAKEDIR})
