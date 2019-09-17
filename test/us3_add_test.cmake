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

#---------------------------------------------------------------------------------------------------
# us3_add_test: Add a test executable.
#
# Usage:
#   us3_add_test(<test source file>)
#---------------------------------------------------------------------------------------------------
function(us3_add_test _source_file)
  get_filename_component(_test_name "${_source_file}" NAME_WE)

  add_executable(${_test_name} ${_source_file})
  target_link_libraries(${_test_name} us3 doctest)
  target_include_directories(${_test_name} PRIVATE ${PROJECT_SOURCE_DIR}/src)
  add_test(${_test_name} ${_test_name})
endfunction()
