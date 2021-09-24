###################################################################################################
# Copyright (c) 2019 Marcus Geelnard
#
# This software is provided 'as-is', without any express or implied warranty. In no event will the
# authors be held liable for any damages arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose, including commercial
# applications, and to alter it and redistribute it freely, subject to the following restrictions:
#
#  1. The origin of this software must not be misrepresented; you must not claim that you wrote
#     the original software. If you use this software in a product, an acknowledgment in the
#     product documentation would be appreciated but is not required.
#
#  2. Altered source versions must be plainly marked as such, and must not be misrepresented as
#     being the original software.
#
#  3. This notice may not be removed or altered from any source distribution.
###################################################################################################

cmake_minimum_required(VERSION 3.4)
project(microS3)

# Create a compilation command database (for LLVM-based linting etc).
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# By default we build shared libs if the global BUILD_SHARED_LIBS flag is set.
set(_us3_build_shared_libs_default OFF)
if(BUILD_SHARED_LIBS)
  set(_us3_build_shared_libs_default ON)
endif()

# Build options for the microS3 project.
option(US3_ENABLE_TESTS         "microS3: Enable unit tests" ON)
option(US3_ENABLE_TOOLS         "microS3: Enable tools" ON)
option(US3_ENABLE_SYSTEM_CRYPTO "microS3: Use system crypto libs when available" OFF)
option(US3_BUILD_SHARED_LIBS    "microS3: Build shared libs" ${_us3_build_shared_libs_default})

if(US3_ENABLE_TESTS)
  enable_testing()
endif()

if(MINGW)
  # For ease of deployment, statically link the standard libraries when using MinGW.
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
endif()

if(MSVC)
  # Avoid CRT secure warnings when using MSVC.
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# We build everything against the C++03 standard (since CMake does not support 3 as a standard, we
# use 98 which is close enough and for most purposes identical).
set(CMAKE_CXX_STANDARD 98)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# External dependencies (doctest).
add_subdirectory(ext)

# The us3 library.
add_subdirectory(lib)

# Tools.
if(US3_ENABLE_TOOLS)
  add_subdirectory(tools)
endif()

# API documentation.
find_package(Doxygen)
if(DOXYGEN_FOUND AND NOT (${CMAKE_VERSION} VERSION_LESS "3.9.0"))
  set(DOXYGEN_PROJECT_NAME microS3)
  set(DOXYGEN_OUTPUT_DIRECTORY apidoc)
  set(DOXYGEN_GENERATE_HTML YES)
  set(DOXYGEN_OPTIMIZE_FOR_C YES)
  set(DOXYGEN_QUIET YES)
  set(DOXYGEN_WARN_IF_UNDOCUMENTED NO)
  doxygen_add_docs(apidoc
                   include
                   COMMENT "Generating API documentation with Doxygen")
endif()
