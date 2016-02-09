#=======================================================================================================================
#
#   Copyright 2011, 2012, 2013, 2014, 2015, 2016 Institut fuer Neuroinformatik, Ruhr-Universitaet Bochum, Germany
# 
#   This file is part of cedar.
#
#   cedar is free software: you can redistribute it and/or modify it under
#   the terms of the GNU Lesser General Public License as published by the
#   Free Software Foundation, either version 3 of the License, or (at your
#   option) any later version.
#
#   cedar is distributed in the hope that it will be useful, but WITHOUT ANY
#   WARRANTY; without even the implied warranty of MERCHANTABILITY or
#   FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
#   License for more details.
#
#   You should have received a copy of the GNU Lesser General Public License
#   along with cedar. If not, see <http://www.gnu.org/licenses/>.
#
#=======================================================================================================================
#
#   Institute:   Ruhr-Universitaet Bochum
#                Institut fuer Neuroinformatik
#
#   File:        CMakeLists.txt
#
#   Maintainer:  Oliver Lomp
#   Email:       oliver.lomp@ini.ruhr-uni-bochum.de
#   Date:        2016 02 09
#
#   Description: Macros for the build-system of the collective plugin used at the INI.
#
#   Credits:
#
#=======================================================================================================================


## General setup & linking to cedar ##
set(CEDAR_BUILD_DIR build)
set(CEDAR_LIB_DIR lib)
set(Boost_Added Off)

# Read the user's settings
file(GLOB conf_check "project.conf")

# if the config file does not exist, copy the example file
if (NOT conf_check)
  file(GLOB conf_example_check "project.conf.example")
  if (conf_example_check)
    print_warning("Configuration file not found. Using example file, please check correctness of the paths!")
    configure_file("project.conf.example" "${CMAKE_CURRENT_SOURCE_DIR}/project.conf" COPYONLY)
  else()
    print_fatal_error("Neither the project.conf nor the project.conf.example file could be found. Please provide configuration files!")
  endif()
endif()

# include the settings from the config fike
include("project.conf")

if (CMAKE_BUILD_TYPE MATCHES "debug")
  add_definitions(-DDEBUG)
endif()

# include cedar variables/directories when CEDAR_HOME is specified
if(CEDAR_HOME)
  print_message("Using local cedar version in ${CEDAR_HOME}")
  # Add include directories
  include_directories("${CEDAR_HOME}" "${CEDAR_HOME}/${CEDAR_BUILD_DIR}" "${CMAKE_CURRENT_BINARY_DIR}")

  # Add link directories
  link_directories("${CEDAR_HOME}/${CEDAR_LIB_DIR}")
  
  # includes and libraries of external dependencies
  include("${CEDAR_HOME}/${CEDAR_BUILD_DIR}/cedar_configuration.cmake")
  
# include cedar variables/directories when CEDAR_HOME_INSTALLED is specified
elseif(CEDAR_HOME_INSTALLED)
  print_message("Using installed cedar version in ${CEDAR_HOME_INSTALLED}")
  # Add include directories
  include_directories("${CEDAR_HOME_INSTALLED}/include" "${CMAKE_CURRENT_BINARY_DIR}")

  # Add link directories
  link_directories("${CEDAR_HOME_INSTALLED}/lib")

  # includes and libraries of external dependencies
  include("${CEDAR_HOME_INSTALLED}/share/cedar/cedar_configuration.cmake")
endif(CEDAR_HOME)

# find OpenCV -- this is necessary because the opencv script sets some internal things (on windows)
set (OpenCV_DIR ${CEDAR_OPENCV_CMAKE_DIR})
find_package(OpenCV REQUIRED)
if (NOT OpenCV_FOUND)
  print_warning("Could not find opencv. You may experience problems linking to this library.")
endif ()


# Add some platform-specific definitions  
if (CMAKE_COMPILER_IS_GNUCC)
  add_definitions(-Wall -Wextra)
  add_definitions(-DGNUCC -D__GNUCC__ -D__GCC__ -DGCC)
  add_definitions(-std=gnu++0x)
elseif (MSVC)
  add_definitions(-W2)
  add_definitions(-DMSVC)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_definitions(-std=c++11)
endif (CMAKE_COMPILER_IS_GNUCC)
