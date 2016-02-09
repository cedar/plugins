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
#   Date:        2016 02 05
#
#   Description: Build-system for the collective plugin used at the INI.
#
#   Credits:
#
#=======================================================================================================================

cmake_minimum_required(VERSION 2.8)

set(SELECTED_BUILDSET "buildset.cmake")
include_directories(${CMAKE_SOURCE_DIR})

include(${CMAKE_SOURCE_DIR}/cmake/macros.cmake)

include(${CMAKE_SOURCE_DIR}/cmake/declaration_macros.cmake)

include(${CMAKE_SOURCE_DIR}/cmake/setup.cmake)


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

configure_file(${CMAKE_SOURCE_DIR}/cmake/plugin.cpp.in plugin.cpp)
list(APPEND PLUGIN_SOURCE_FILES ${CMAKE_CURRENT_BINARY_DIR}/plugin.cpp)

## generate the icons.qrc file ##

configure_file(${CMAKE_SOURCE_DIR}/cmake/icons.qrc.in icons.qrc)
qt_add_resources(compiled_resource_paths ${CMAKE_CURRENT_BINARY_DIR}/icons.qrc)
list(APPEND PLUGIN_SOURCE_FILES ${compiled_resource_paths})

## Generate the plugin target ##

# TODO
add_library(${PLUGIN_TARGET_NAME} SHARED ${PLUGIN_SOURCE_FILES})

target_link_libraries(${PLUGIN_TARGET_NAME} ${PLUGIN_LINKED_LIBRARIES})


## Generate the documentation ##

# generate the welcome text #
file(READ ${CMAKE_SOURCE_DIR}/cmake/readme_src/welcome.md WELCOME_TEXT)

# generate the step list #

list(REMOVE_DUPLICATES CATEGORIES)
list(SORT CATEGORIES)

foreach (category ${CATEGORIES})
  set(STEP_LIST_TEXT "${STEP_LIST_TEXT}\#\# ${category} \#\#\n\n")
  list(SORT known_steps)
  foreach (FULL_CLASS_NAME ${known_steps})
    NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
    EXTRACT_CLASS_NAME(${FULL_CLASS_NAME})
    set(step_category ${CATEGORY_${NORMALIZED_CLASS_NAME}})
    set(description ${DESCRIPTION_${NORMALIZED_CLASS_NAME}})
    if (NOT description)
      set(description "no description.")
    endif()
    
    if (category STREQUAL step_category)
      set(STEP_LIST_TEXT "${STEP_LIST_TEXT}**${CLASS_NAME}**: ${description}\n\n")
    endif()
  endforeach()
  set(STEP_LIST_TEXT "${STEP_LIST_TEXT}\n\n")
endforeach()

# generate the descriptions for external libraries #

file(GLOB_RECURSE ext_descriptions "${CMAKE_SOURCE_DIR}/external_libraries/*.md")
list(SORT ext_descriptions)
foreach (description ${ext_descriptions})
  file(READ ${description} description_md)
  set(EXTERNAL_LIBRARIES_TEXT "${EXTERNAL_LIBRARIES_TEXT}${description_md}")
endforeach()

configure_file(${CMAKE_SOURCE_DIR}/cmake/readme_src/readme.md.in ${CMAKE_SOURCE_DIR}/readme.md)
