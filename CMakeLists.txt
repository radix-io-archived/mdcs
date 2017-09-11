#-----------------------------------------------------------#
# Copyright (C) 2017 University of Chicago.
# See COPYRIGHT notice in top-level directory.
#-----------------------------------------------------------#
cmake_minimum_required (VERSION 2.8)

#-----------------------------------------------------------#
# Starting project definition
#-----------------------------------------------------------#
project(MDCS)

set(CMAKE_C_FLAGS "-g -Wall -Wextra -Wno-unused-parameter")
#-----------------------------------------------------------#
# Set the location of CMake Modules
#-----------------------------------------------------------#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} 
		"${MCDS_SOURCE_DIR}/cmake/Modules/")

if(NOT DEFINED CMAKE_INSTALL_PREFIX)
	set(CMAKE_INSTALL_PREFIX /usr/local)
endif(NOT DEFINED CMAKE_INSTALL_PREFIX)

find_package(ABT)
if(${ABT_FOUND})
  include_directories(${ABT_INCLUDE_DIR})
endif()

find_package(MERCURY)
if(${MERCURY_FOUND})
  find_package(BMI)
  if(${BMI_FOUND})
    include_directories(${BMI_INCLUDE_DIRS})
    set(HAS_BMI ON)
  endif()
  find_package(CCI)
  if(${CCI_FOUND})
    include_directories(${CCI_INCLUDE_DIRS})
    set(HAS_CCI ON)
  endif()
  if(NOT (${BMI_FOUND} OR ${CCI_FOUND}))
    message(FATAL_ERROR "No transport method (BMI or CCI) found")
  endif()
  find_package(Boost REQUIRED)
  include_directories(${MERCURY_INCLUDE_DIRS})
  include_directories(${Boost_INCLUDE_DIRS})
endif()

if(${MERCURY_FOUND} AND ${ABT_FOUND})
  find_package(ABT_SNOOZER)
  if(${ABT_SNOOZER_FOUND})
    find_package(MARGO)
    include_directories(${MARGO_INCLUDE_DIRS})
    include_directories(${ABT_SNOOZER_INCLUDE_DIRS})
  endif()
endif()

#-----------------------------------------------------------#
include_directories(${MCDS_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR})

#-----------------------------------------------------------#
# Set subdirectories for the compilation process
#-----------------------------------------------------------#
add_subdirectory(lib)

