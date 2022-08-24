#  Copyright (c) 2018, Facebook, Inc.
#  All rights reserved.
#
#  This source code is licensed under the BSD-style license found in the
#  LICENSE file in the root directory of this source tree.
#
# CMake package configuration file for fizz
#
# Defines the target "fizz::fizz"
# Add this to your target_link_libraries() call to depend on fizz::fizz.
#
# Also sets the variables FIZZ_INCLUDE_DIR and FIZZ_LIBRARIES.
# However, in most cases using the fizz::fizz target is sufficient,
# and you won't need these variables.
#


####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was fizz-config.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################
#"${CMAKE_SOURCE_DIR}/proxygen/proxygen/_build/deps/include"
set_and_check(FIZZ_INCLUDE_DIR "${PACKAGE_PREFIX_DIR}/include")
#set_and_check(FIZZ_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/proxygen/proxygen/_build/deps/include")
#set_and_check(FIZZ_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/proxygen/proxygen/_build/deps/fizz")
set_and_check(FIZZ_CMAKE_DIR "${PACKAGE_PREFIX_DIR}/lib/cmake/folly")

message(STATUS "fizz-config.cmake.in :: FIZZ_INCLUDE_DIR = ${FIZZ_INCLUDE_DIR}")

message(STATUS "fizz-config.cmake.in :: FIZZ_CMAKE_DIR = ${FIZZ_CMAKE_DIR}")

if (NOT TARGET fizz::fizz)
  include("${FIZZ_CMAKE_DIR}/fizz-targets.cmake")
endif()

set(FIZZ_LIBRARIES fizz::fizz)

include(CMakeFindDependencyMacro)
find_dependency(Sodium)
find_dependency(folly CONFIG)
find_dependency(ZLIB)

if (NOT fizz_FIND_QUIETLY)
  message(STATUS "Found fizz: ${PACKAGE_PREFIX_DIR}")
endif()
