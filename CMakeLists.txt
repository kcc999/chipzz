cmake_minimum_required(VERSION 3.16)

project(chipzz LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(chipzz
  src/main.cc
  src/emu.cc
)

target_include_directories(chipzz PRIVATE src)

# Raylib resolution strategy:
# 1) CMake package (preferred, supports static/shared depending on install)
# 2) pkg-config (common on Linux/BSD)
# 3) Manual header + library lookup (tries static first, then shared)
find_package(raylib CONFIG QUIET)
if(raylib_FOUND)
  target_link_libraries(chipzz PRIVATE raylib)
else()
  find_package(PkgConfig QUIET)
  if(PkgConfig_FOUND)
    pkg_check_modules(RAYLIB QUIET raylib)
  endif()

  if(RAYLIB_FOUND)
    target_include_directories(chipzz PRIVATE ${RAYLIB_INCLUDE_DIRS})
    target_link_directories(chipzz PRIVATE ${RAYLIB_LIBRARY_DIRS})
    target_link_libraries(chipzz PRIVATE ${RAYLIB_LIBRARIES})
    target_compile_options(chipzz PRIVATE ${RAYLIB_CFLAGS_OTHER})
  else()
    find_path(RAYLIB_INCLUDE_DIR raylib.h
      PATH_SUFFIXES include
      PATHS
        /usr
        /usr/local
        /opt/homebrew
        /opt/local
    )

    find_library(RAYLIB_SHARED_LIBRARY
      NAMES raylib
      PATH_SUFFIXES lib lib64
      PATHS
        /usr
        /usr/local
        /opt/homebrew
        /opt/local
    )

    find_library(RAYLIB_STATIC_LIBRARY
      NAMES raylib_static raylib
      PATH_SUFFIXES lib lib64
      PATHS
        /usr
        /usr/local
        /opt/homebrew
        /opt/local
    )

    if(RAYLIB_INCLUDE_DIR AND RAYLIB_STATIC_LIBRARY)
      target_include_directories(chipzz PRIVATE ${RAYLIB_INCLUDE_DIR})
      target_link_libraries(chipzz PRIVATE ${RAYLIB_STATIC_LIBRARY})
    elseif(RAYLIB_INCLUDE_DIR AND RAYLIB_SHARED_LIBRARY)
      target_include_directories(chipzz PRIVATE ${RAYLIB_INCLUDE_DIR})
      target_link_libraries(chipzz PRIVATE ${RAYLIB_SHARED_LIBRARY})
    else()
      message(FATAL_ERROR
        "raylib not found. Install raylib and expose it via CMake config, pkg-config, "
        "or standard include/lib paths.")
    endif()
  endif()
endif()

if(WIN32)
  target_link_libraries(chipzz PRIVATE winmm gdi32 opengl32)
elseif(APPLE)
  find_library(COCOA_FRAMEWORK Cocoa REQUIRED)
  find_library(IOKIT_FRAMEWORK IOKit REQUIRED)
  find_library(COREVIDEO_FRAMEWORK CoreVideo REQUIRED)
  target_link_libraries(chipzz PRIVATE
    ${COCOA_FRAMEWORK}
    ${IOKIT_FRAMEWORK}
    ${COREVIDEO_FRAMEWORK}
  )
else()
  target_link_libraries(chipzz PRIVATE m pthread dl rt X11)
endif()
