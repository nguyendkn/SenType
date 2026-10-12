cmake_minimum_required(VERSION 3.20)
if(APPLE)  # must precede project(): universal binary, macOS 11+
  set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "")
  set(CMAKE_OSX_DEPLOYMENT_TARGET 11.0 CACHE STRING "")
endif()
project(sentype VERSION 0.1.0 LANGUAGES CXX)

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
