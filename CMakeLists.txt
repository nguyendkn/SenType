cmake_minimum_required(VERSION 3.20)
if(APPLE)  # must precede project(): universal binary, macOS 11+
  set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "")
  set(CMAKE_OSX_DEPLOYMENT_TARGET 11.0 CACHE STRING "")
endif()
# Single source of truth: ./VERSION holds MAJOR.MINOR. Release builds pass the tag version
# (v<MAJOR.MINOR>.<YYYYMMDD><INDEX> minus the "v") as -DSENTYPE_VERSION; packages, plist and installer use it.
file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/VERSION" SENTYPE_BASE_VERSION LIMIT_COUNT 1)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/VERSION")
if(NOT SENTYPE_BASE_VERSION MATCHES "^[0-9]+\\.[0-9]+$")
  message(FATAL_ERROR "VERSION must contain MAJOR.MINOR, got '${SENTYPE_BASE_VERSION}'")
endif()
project(sentype VERSION ${SENTYPE_BASE_VERSION}.0 LANGUAGES CXX)
set(SENTYPE_VERSION "${PROJECT_VERSION}" CACHE STRING "full release version, e.g. 0.1.202610120")
if(NOT SENTYPE_VERSION STREQUAL PROJECT_VERSION AND NOT SENTYPE_VERSION MATCHES "^${SENTYPE_BASE_VERSION}\\.[0-9]+$")
  message(FATAL_ERROR "SENTYPE_VERSION '${SENTYPE_VERSION}' must look like ${SENTYPE_BASE_VERSION}.<YYYYMMDD><INDEX>")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Exactly one frontend per build tree; "none" builds only core + its tests.
set(SENTYPE_FRONTEND "none" CACHE STRING "none | linux-fcitx5 | macos | windows")
set_property(CACHE SENTYPE_FRONTEND PROPERTY STRINGS none linux-fcitx5 macos windows)

if(MSVC)
  add_compile_options(/utf-8)  # sources contain UTF-8 Vietnamese literals
  # One static CRT for every target (core libs, TSF DLL, tests): the DLL ships without the VC++ redistributable,
  # and mixing /MD libs into a /MT DLL fails with LNK2038.
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif(MINGW)
  add_link_options(-static)  # self-contained .exe/.dll: no libstdc++/libgcc/winpthread DLLs to ship
endif()

# ASan + UBSan for every target (core, tests, fuzz): -DSENTYPE_SANITIZE=ON
option(SENTYPE_SANITIZE "build with AddressSanitizer + UBSan" OFF)
if(SENTYPE_SANITIZE AND NOT MSVC)
  add_compile_options(-fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

# libFuzzer target (clang only): instruments core too so the fuzzer is coverage guided.
option(SENTYPE_FUZZ "build the libFuzzer target (clang only)" OFF)
if(SENTYPE_FUZZ)
  add_compile_options(-fsanitize=fuzzer-no-link)
endif()

enable_testing()
add_subdirectory(core)

if(SENTYPE_FRONTEND STREQUAL "linux-fcitx5")
  if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr CACHE PATH "" FORCE)  # Debian layout
  endif()
  add_subdirectory(frontends/linux-fcitx5)
  include(frontends/linux-fcitx5/cmake/packaging.cmake)
elseif(SENTYPE_FRONTEND STREQUAL "macos")
  enable_language(OBJCXX)
  add_subdirectory(frontends/macos)
elseif(SENTYPE_FRONTEND STREQUAL "windows")
  add_subdirectory(frontends/windows)
endif()
