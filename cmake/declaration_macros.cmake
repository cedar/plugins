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
  EXTRACT_CLASS_NAME(${FULL_CLASS_NAME})
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
      list(APPEND CATEGORIES ${arg})
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
