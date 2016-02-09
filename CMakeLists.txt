#=======================================================================================================================
#
#   Copyright 2011, 2012, 2013, 2014 Institut fuer Neuroinformatik, Ruhr-Universitaet Bochum, Germany
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
#   Date:        2016 02 05
#
#   Description: Build-system for the collective plugin used at the INI.
#
#   Credits:
#
#=======================================================================================================================

cmake_minimum_required(VERSION 2.8)

set(PLUGIN_TARGET_NAME "plugin")
set(CEDAR_BUILD_DIR build)
set(CEDAR_LIB_DIR lib)
set(Boost_Added Off)
set(SELECTED_BUILDSET "buildset.cmake")
include_directories(${CMAKE_SOURCE_DIR})


## Wrapper around QT4/5 macros ##

macro(qt_wrap_cpp)
  if(QT_VERSION GREATER 4)
    qt5_wrap_cpp(${ARGN})
  else(QT_VERSION GREATER 4)
    qt4_wrap_cpp(${ARGN})
  endif(QT_VERSION GREATER 4)
endmacro(qt_wrap_cpp)

macro(qt_add_resources)
  if(QT_VERSION GREATER 4)
    qt5_add_resources(${ARGV})
  else(QT_VERSION GREATER 4)
    qt4_add_resources(${ARGV})
  endif(QT_VERSION GREATER 4)
endmacro(qt_add_resources)

macro(qt_wrap_ui ${ARGV})
  if(QT_VERSION GREATER 4)
    qt5_wrap_ui(${ARGV})
  else(QT_VERSION GREATER 4)
    qt4_wrap_ui(${ARGV})
  endif(QT_VERSION GREATER 4)
endmacro(qt_wrap_ui)

# Some macros for nicer message/warning/error printing

macro(print_warning warning)
  message("!! Warning: ${warning}")
endmacro(print_warning)

macro(print_message msg)
  message("-- ${msg}")
endmacro(print_message)

macro(print_error error)
  message(SEND_ERROR "${error}")
endmacro(print_error)

macro(print_fatal_error error)
  message(FATAL_ERROR "${error}")
endmacro(print_fatal_error)

## General setup & linking to cedar ##

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

## Macro definitions related to the plugin build system ##

macro(NORMALIZE_CLASS_NAME FULL_CLASS_NAME)
  string(REGEX REPLACE "::" "__" NORMALIZED_CLASS_NAME ${FULL_CLASS_NAME})
endmacro(NORMALIZE_CLASS_NAME)

# This is a macro called by step description files.
#
# It automatically generates build information required for the step, namely, the cpp and header files, icon,
# description etc. All but the first (full class name) parameters are optional. Here is an example call with a list of 
# all possible parameters:
# DECLARE_STEP(cedar::proc::steps::YourClass # automatically includes YourClass.cpp and YourClass.h to be compiled
#              MOC # if specified, the class header will be moced.
#              CATEGORY "YourCategory"
#              DESCRIPTION "Describe what the step does."
#              MAINTAINER "Your Name" # to let people know who to contact about this step
#             )
macro(DECLARE_STEP FULL_CLASS_NAME)
  # extract the class name, without the namespace
  string(REGEX REPLACE ".*::([[A-Za-z0-9_]*)$" "\\1" CLASS_NAME ${FULL_CLASS_NAME})
  # normalize the class name so it can be used to declare variables
  NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
  
  # register the step with the list of steps
  list(APPEND known_steps ${FULL_CLASS_NAME})
  
  # append the auto-determined cpp file for the class
  # TODO check if this file exists; if not, don't add it
  list(APPEND "source_files_${NORMALIZED_CLASS_NAME}" "${CMAKE_CURRENT_LIST_DIR}/${CLASS_NAME}.cpp")
  # these headers are used to add them to the generated plugin.cpp file
  list(APPEND "header_files_${NORMALIZED_CLASS_NAME}" "${CMAKE_CURRENT_LIST_DIR}/${CLASS_NAME}.h")
  
  if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/${CLASS_NAME}.svg")
    list(APPEND "icon_${NORMALIZED_CLASS_NAME}" "${CLASS_NAME}.svg")
    list(APPEND "icon_path_${NORMALIZED_CLASS_NAME}" "${CMAKE_CURRENT_LIST_DIR}/${CLASS_NAME}.svg")
  endif()
  
  set(STATE_NONE 0)
  set(STATE_CATEGORY 1)
  set(STATE_DESCRIPTION 2)
  set(STATE_MAINTAINER 3)
  set(STATE_REQUIRES_LIBRARIES 4)
  
  set(CURRENT_STATE ${STATE_NONE})
  
  foreach (arg ${ARGN})
    if (arg STREQUAL "CATEGORY")
      set(CURRENT_STATE ${STATE_CATEGORY})
    elseif (arg STREQUAL "DESCRIPTION")
      set(CURRENT_STATE ${STATE_DESCRIPTION})
    elseif (arg STREQUAL "MAINTAINER")
      set(CURRENT_STATE ${STATE_MAINTAINER})
    elseif (arg STREQUAL "REQUIRES_LIBRARIES" OR arg STREQUAL "REQUIRES_LIBRARY")
      set(CURRENT_STATE ${STATE_REQUIRES_LIBRARIES})
    elseif(arg STREQUAL "MOC")
      list(APPEND "moc_headers_${NORMALIZED_CLASS_NAME}" "${CMAKE_CURRENT_LIST_DIR}/${CLASS_NAME}.h")
    elseif(CURRENT_STATE EQUAL STATE_CATEGORY)
      set("CATEGORY_${NORMALIZED_CLASS_NAME}" ${arg})
    elseif(CURRENT_STATE EQUAL STATE_DESCRIPTION)
      set("DESCRIPTION_${NORMALIZED_CLASS_NAME}" ${arg})
    elseif(CURRENT_STATE EQUAL STATE_MAINTAINER)
      set("MAINTAINER_${NORMALIZED_CLASS_NAME}" ${arg})
    elseif(CURRENT_STATE EQUAL STATE_REQUIRES_LIBRARIES)
      set("REQUIRES_LIBRARIES_${NORMALIZED_CLASS_NAME}" ${arg})
    endif()
  endforeach()
  if(NOT DESCRIPTION_${NORMALIZED_CLASS_NAME})
    set(maintainer_warning "")
    if (MAINTAINER_${NORMALIZED_CLASS_NAME})
      set(maintainer_warning " (maintainer is ${MAINTAINER_${NORMALIZED_CLASS_NAME}})")
    endif()
    print_warning("The step ${FULL_CLASS_NAME} has no description${maintainer_warning}.")
  endif()
endmacro(DECLARE_STEP)

# TODO describe syntax
macro(DECLARE_PLUGIN PLUGIN_NAME)
  set(PLUGIN_TARGET_NAME ${PLUGIN_NAME})
endmacro(DECLARE_PLUGIN)

#
# macro ADD_STEP_SOURCES_TO_BUILD
#
macro(ADD_STEP_SOURCES_TO_BUILD FULL_CLASS_NAME)
  NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
  set(SOURCE_FILES "${source_files_${NORMALIZED_CLASS_NAME}}")
  list(APPEND PLUGIN_SOURCE_FILES ${SOURCE_FILES})
  set(decl " {\n")
  set(decl "${decl}    cedar::proc::ElementDeclarationPtr declaration\n")
  set(decl "${decl}    (\n")
  set(decl "${decl}      new cedar::proc::ElementDeclarationTemplate<${FULL_CLASS_NAME}>(\"${CATEGORY_${NORMALIZED_CLASS_NAME}}\")\n")
  set(decl "${decl}    );\n")
  
  # add description, if set
  set(description ${DESCRIPTION_${NORMALIZED_CLASS_NAME}})
  if (description)
    string(REPLACE "\"" "\\\"" description ${description})
    set(decl "${decl}    declaration->setDescription(\"${description}\");\n")
  endif()
  
  set(icon ${icon_${NORMALIZED_CLASS_NAME}})
  set(icon_path ${icon_path_${NORMALIZED_CLASS_NAME}})
  if (icon)
    set(decl "${decl}    declaration->setIconPath(\":/cedar/plugins/${icon}\");\n")
    set(PLUGIN_ICONS "${PLUGIN_ICONS}     <file alias=\"${icon}\">${icon_path}</file>\n")
  endif()
  
  set(decl "${decl}    plugin->add(declaration);\n")
  set(decl "${decl}  }\n")
  set(PLUGIN_DECLARATIONS "${PLUGIN_DECLARATIONS} ${decl}")
  
  list(LENGTH "moc_headers_${NORMALIZED_CLASS_NAME}" list_length)
  if (list_length GREATER 0)
    qt_wrap_cpp(moc_sources ${moc_headers_${NORMALIZED_CLASS_NAME}})
    list(APPEND PLUGIN_SOURCE_FILES ${moc_sources})
  endif()
  
  if (REQUIRES_LIBRARIES_${NORMALIZED_CLASS_NAME})
    list(APPEND PLUGIN_REQUIRES_LIBRARIES ${REQUIRES_LIBRARIES_${NORMALIZED_CLASS_NAME}})
  endif()
  
  foreach (header ${header_files_${NORMALIZED_CLASS_NAME}})
    set(PLUGIN_INCLUDE_FILES "${PLUGIN_INCLUDE_FILES} \#include \"${header}\"\n")
  endforeach()
endmacro(ADD_STEP_SOURCES_TO_BUILD)

# TODO describe syntax
macro(ADD_TO_PLUGIN)
  set(STATE_NONE 0)
  set(STATE_STEPS 1)
  set(STATE_CATEGORIES 2)
  set(CURRENT_STATE ${STATE_NONE})
  foreach (arg ${ARGN})
    if (arg STREQUAL "STEPS" OR arg STREQUAL "STEP")
      set(CURRENT_STATE ${STATE_STEPS})
    elseif (arg STREQUAL "CATEGORIES" OR arg STREQUAL "CATEGORY")
      set(CURRENT_STATE ${STATE_CATEGORIES})
    elseif(arg STREQUAL "ALL_STEPS")
      foreach (FULL_CLASS_NAME ${known_steps})
        NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
        ADD_STEP_SOURCES_TO_BUILD(${FULL_CLASS_NAME})
      endforeach()
    elseif (CURRENT_STATE EQUAL ${STATE_NONE})
      print_error("Syntax error in ADD_TO_PLUGIN: prefix with STEPS, CATEGORIES, ...")
    elseif (CURRENT_STATE EQUAL ${STATE_STEPS})
      ADD_STEP_SOURCES_TO_BUILD(${arg})
    elseif (CURRENT_STATE EQUAL ${STATE_CATEGORIES})
      foreach (FULL_CLASS_NAME ${known_steps})
        NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
        set(CATEGORY ${CATEGORY_${NORMALIZED_CLASS_NAME}})
        if (${CATEGORY} STREQUAL arg)
          ADD_STEP_SOURCES_TO_BUILD(${FULL_CLASS_NAME})
        endif()
      endforeach()
    endif()
  endforeach()
endmacro(ADD_TO_PLUGIN)

## Find and read all step declarations ##

list(APPEND code_directories steps)

# go through all directories that contain code
foreach (directory ${code_directories})
  # in each directory, look for cmake files; these should contain declarations
  file(GLOB_RECURSE files "${CMAKE_SOURCE_DIR}/${directory}/*.cmake")
  foreach (file ${files})
    # load the declaration
    include(${file})
  endforeach()
endforeach()

## build a list of all source files and dependencies based on the build set ##

include(${SELECTED_BUILDSET})

## Find and compile all required external libraries ##
if (PLUGIN_REQUIRES_LIBRARIES)
  list(REMOVE_DUPLICATES PLUGIN_REQUIRES_LIBRARIES)
  
  foreach (required_library ${PLUGIN_REQUIRES_LIBRARIES})
    print_message("Looking for external library ${required_library}")
    set(include_file "${CMAKE_SOURCE_DIR}/external_libraries/${required_library}.cmake")
    if (EXISTS "${include_file}")
      include (${include_file})
      
      string(TOUPPER ${required_library} required_library_upper)
      if (${required_library_upper}_FOUND)
        message("DEBUG: incl: ${${required_library_upper}_INCLUDE}")
        include_directories(${${required_library_upper}_INCLUDE})
        list(APPEND PLUGIN_LINKED_LIBRARIES ${${required_library_upper}_LIBS})
      else()
        print_fatal_error("Required library ${required_library} was not found.")
      endif()
    else()
      print_fatal_error("${required_library}.cmake not found in external_libraries folder.")
    endif()
  endforeach()
endif()

## generate the plugin.cpp file ##

configure_file(${CMAKE_SOURCE_DIR}/plugin.cpp.in plugin.cpp)
list(APPEND PLUGIN_SOURCE_FILES ${CMAKE_CURRENT_BINARY_DIR}/plugin.cpp)

## generate the icons.qrc file ##

configure_file(${CMAKE_SOURCE_DIR}/icons.qrc.in icons.qrc)
qt_add_resources(compiled_resource_paths ${CMAKE_CURRENT_BINARY_DIR}/icons.qrc)
list(APPEND PLUGIN_SOURCE_FILES ${compiled_resource_paths})

## Generate the plugin target ##

# TODO
add_library(${PLUGIN_TARGET_NAME} SHARED ${PLUGIN_SOURCE_FILES})

target_link_libraries(${PLUGIN_TARGET_NAME} ${PLUGIN_LINKED_LIBRARIES})
