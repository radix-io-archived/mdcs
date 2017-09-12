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
		"${MDCS_SOURCE_DIR}/cmake/Modules/")

if(NOT DEFINED CMAKE_INSTALL_PREFIX)
	set(CMAKE_INSTALL_PREFIX /usr/local)
endif(NOT DEFINED CMAKE_INSTALL_PREFIX)

find_package(ABT REQUIRED)
find_package(MERCURY REQUIRED)
find_package(Boost REQUIRED)
find_package(MARGO REQUIRED)

include_directories(${MERCURY_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${ABT_INCLUDE_DIR})
include_directories(${MARGO_INCLUDE_DIRS})
#-----------------------------------------------------------#
include_directories(${MDCS_SOURCE_DIR})
include_directories(${MDCS_SOURCE_DIR}/include)
include_directories(${CMAKE_BINARY_DIR})

#-----------------------------------------------------------#
# Set subdirectories for the compilation process
#-----------------------------------------------------------#
add_subdirectory(lib)
add_subdirectory(test)

install(FILES "${CMAKE_BINARY_DIR}/lib/libmdcs.a" DESTINATION lib)
install(DIRECTORY "include/mdcs" DESTINATION include)
