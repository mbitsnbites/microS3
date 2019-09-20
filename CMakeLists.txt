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

cmake_minimum_required(VERSION 3.5)
project(microS3)

enable_testing()

# The us3 library.
add_subdirectory(lib)

# Tests.
add_subdirectory(test)

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
