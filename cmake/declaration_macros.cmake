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
#              DEPRECATED_NAME some::old::Name
#              [or: DEPRECATED_NAMES some::old::Name some::other::old::Name]
#              REQUIRES_LIBRARY some_lib_from_the_external_libraries_folder
#              [or: REQUIRES_LIBRARIES lib1 lib2 ...]
#              REQUIRES_DATA_STRUCTURES some::data::Structure some::other::DataStructure
#             )
macro(DECLARE_STEP FULL_CLASS_NAME)
  # extract the class name, without the namespace
  EXTRACT_CLASS_NAME(${FULL_CLASS_NAME})
  # normalize the class name so it can be used to declare variables
  NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
  
  # register the step with the list of steps
  list(APPEND known_steps ${FULL_CLASS_NAME})
  
  # append the auto-determined cpp and h file for the class
  DECLARE_CLASS_FILES(${FULL_CLASS_NAME})
  
  set(STATE_NONE 0)
  set(STATE_CATEGORY 1)
  set(STATE_DESCRIPTION 2)
  set(STATE_MAINTAINER 3)
  set(STATE_REQUIRES_LIBRARIES 4)
  set(STATE_DEPRECATED_NAMES 5)
  set(STATE_REQUIRES_DATA_STRUCTURES 6)
  
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
    elseif (arg STREQUAL "DEPRECATED_NAME" OR arg STREQUAL "DEPRECATED_NAMES")
      set(CURRENT_STATE ${STATE_DEPRECATED_NAMES})
    elseif (arg STREQUAL "REQUIRES_DATA_STRUCTURE" OR arg STREQUAL "REQUIRES_DATA_STRUCTURES")
      set(CURRENT_STATE ${STATE_REQUIRES_DATA_STRUCTURES})
    elseif(arg STREQUAL "MOC")
      DECLARE_MOC_HEADERS(${FULL_CLASS_NAME})
    elseif(CURRENT_STATE EQUAL STATE_CATEGORY)
      set("CATEGORY_${NORMALIZED_CLASS_NAME}" ${arg})
      list(APPEND CATEGORIES ${arg})
    elseif(CURRENT_STATE EQUAL STATE_DESCRIPTION)
      DECLARE_CLASS_DESCRIPTION(${FULL_CLASS_NAME} ${arg})
    elseif(CURRENT_STATE EQUAL STATE_MAINTAINER)
      set("MAINTAINER_${NORMALIZED_CLASS_NAME}" ${arg})
    elseif(CURRENT_STATE EQUAL STATE_REQUIRES_LIBRARIES)
      DECLARE_REQUIRED_LIBRARY(${FULL_CLASS_NAME} ${arg})
    elseif(CURRENT_STATE EQUAL STATE_DEPRECATED_NAMES)
      DECLARE_DEPRECATED_NAME(${FULL_CLASS_NAME} ${arg})
    elseif(CURRENT_STATE EQUAL STATE_REQUIRES_DATA_STRUCTURES)
      DECLARE_DATA_STRUCTURE_DEPENDENCY(${FULL_CLASS_NAME} ${arg})
    endif()
  endforeach()
  
  CHECK_DESCRIPTION(${FULL_CLASS_NAME} ${NORMALIZED_CLASS_NAME} "step")
endmacro(DECLARE_STEP)

# This is a macro called by kernel description files.
#
# It automatically generates build information required for the kernel, namely, the cpp and header files, icon,
# description etc. All but the first (full class name) parameters are optional.
# TODO this macro is highly redundant with DECLARE_STEP
macro(DECLARE_KERNEL FULL_CLASS_NAME)
  # extract the class name, without the namespace
  EXTRACT_CLASS_NAME(${FULL_CLASS_NAME})
  # normalize the class name so it can be used to declare variables
  NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
  
  # register the step with the list of steps
  list(APPEND known_kernels ${FULL_CLASS_NAME})
  
  # append the auto-determined cpp and h file for the class
  DECLARE_CLASS_FILES(${FULL_CLASS_NAME})
  
  set(STATE_NONE 0)
  set(STATE_DESCRIPTION 1)
  set(STATE_DEPRECATED_NAMES 2)
  
  set(CURRENT_STATE ${STATE_NONE})
  
  foreach (arg ${ARGN})
    if (arg STREQUAL "DESCRIPTION")
      set(CURRENT_STATE ${STATE_DESCRIPTION})
    elseif(arg STREQUAL "MOC")
      DECLARE_MOC_HEADERS(${FULL_CLASS_NAME})
    elseif (arg STREQUAL "DEPRECATED_NAME" OR arg STREQUAL "DEPRECATED_NAMES")
      set(CURRENT_STATE ${STATE_DEPRECATED_NAMES})
    elseif(CURRENT_STATE EQUAL STATE_DESCRIPTION)
      DECLARE_CLASS_DESCRIPTION(${FULL_CLASS_NAME} ${arg})
    elseif(CURRENT_STATE EQUAL STATE_DEPRECATED_NAMES)
      DECLARE_DEPRECATED_NAME(${FULL_CLASS_NAME} ${arg})
    endif()
  endforeach()
  
  CHECK_DESCRIPTION(${FULL_CLASS_NAME} ${NORMALIZED_CLASS_NAME} "kernel")
endmacro(DECLARE_KERNEL)

# This is a macro called by kernel description files.
#
# It automatically generates build information required for the kernel, namely, the cpp and header files, icon,
# description etc. All but the first (full class name) parameters are optional.
# TODO this macro is highly redundant with DECLARE_STEP
macro(DECLARE_DATA_STRUCTURE FULL_CLASS_NAME)
  # extract the class name, without the namespace
  EXTRACT_CLASS_NAME(${FULL_CLASS_NAME})
  # normalize the class name so it can be used to declare variables
  NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
  
  # register the step with the list of steps
  list(APPEND known_data_structures ${FULL_CLASS_NAME})
  
  # append the auto-determined cpp and h file for the class
  DECLARE_CLASS_FILES(${FULL_CLASS_NAME})
    
  set(STATE_NONE 0)
  set(STATE_DESCRIPTION 1)
  set(STATE_DEPRECATED_NAMES 2)
  set(STATE_REQUIRED_LIBRARIES 3)
  
  set(CURRENT_STATE ${STATE_NONE})
  
  foreach (arg ${ARGN})
    if (arg STREQUAL "DESCRIPTION")
      set(CURRENT_STATE ${STATE_DESCRIPTION})
    elseif(arg STREQUAL "MOC")
      DECLARE_MOC_HEADERS(${FULL_CLASS_NAME})
    elseif (arg STREQUAL "DEPRECATED_NAME" OR arg STREQUAL "DEPRECATED_NAMES")
      set(CURRENT_STATE ${STATE_DEPRECATED_NAMES})
    elseif (arg STREQUAL "REQUIRES_LIBRARY" OR arg STREQUAL "REQUIRES_LIBRARIES")
      set(CURRENT_STATE ${STATE_REQUIRED_LIBRARIES})
    elseif(CURRENT_STATE EQUAL STATE_DESCRIPTION)
      DECLARE_CLASS_DESCRIPTION(${FULL_CLASS_NAME} ${arg})
    elseif(CURRENT_STATE EQUAL STATE_DEPRECATED_NAMES)
      DECLARE_DEPRECATED_NAME(${FULL_CLASS_NAME} ${arg})
    elseif(CURRENT_STATE EQUAL STATE_REQUIRED_LIBRARIES)
      DECLARE_REQUIRED_LIBRARY(${FULL_CLASS_NAME} ${arg})
    endif()
  endforeach()
  
  CHECK_DESCRIPTION(${FULL_CLASS_NAME} ${NORMALIZED_CLASS_NAME} "kernel")
endmacro(DECLARE_DATA_STRUCTURE)

macro(CHECK_DESCRIPTION FULL_CLASS_NAME NORMALIZED_CLASS_NAME THING)
  if(NOT DESCRIPTION_${NORMALIZED_CLASS_NAME})
    set(maintainer_warning "")
    if (MAINTAINER_${NORMALIZED_CLASS_NAME})
      set(maintainer_warning " (maintainer is ${MAINTAINER_${NORMALIZED_CLASS_NAME}})")
    endif()
    print_warning("The ${THING} ${FULL_CLASS_NAME} has no description${maintainer_warning}.")
  endif()
endmacro(CHECK_DESCRIPTION)

# TODO describe syntax
macro(DECLARE_PLUGIN PLUGIN_NAME)
  set(PLUGIN_TARGET_NAME ${PLUGIN_NAME})
endmacro(DECLARE_PLUGIN)

# TODO describe syntax
macro(ADD_TO_PLUGIN)
  set(STATE_NONE 0)
  set(STATE_STEPS 1)
  set(STATE_CATEGORIES 2)
  set(STATE_KERNELS 3)
  set(STATE_DATA_STRUCTURES 4)
  
  set(CURRENT_STATE ${STATE_NONE})
  foreach (arg ${ARGN})
    if (${arg} STREQUAL "STEPS" OR ${arg} STREQUAL "STEP")
      set(CURRENT_STATE ${STATE_STEPS})
    elseif (arg STREQUAL "KERNELS" OR arg STREQUAL "KERNEL")
      set(CURRENT_STATE ${STATE_KERNELS})
    elseif (${arg} STREQUAL "CATEGORIES" OR ${arg} STREQUAL "CATEGORY")
      set(CURRENT_STATE ${STATE_CATEGORIES})
    elseif (${arg} STREQUAL "DATA_STRUCTURE" OR ${arg} STREQUAL "DATA_STRUCTURES")
      set(CURRENT_STATE ${STATE_DATA_STRUCTURES})
    elseif(arg STREQUAL "ALL_STEPS")
      foreach (FULL_CLASS_NAME ${known_steps})
        NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
        ADD_STEP_SOURCES_TO_BUILD(${FULL_CLASS_NAME})
      endforeach()
    elseif(arg STREQUAL "ALL_KERNELS")
      foreach (FULL_CLASS_NAME ${known_kernels})
        NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
        ADD_KERNEL_SOURCES_TO_BUILD(${FULL_CLASS_NAME})
      endforeach()
    elseif(arg STREQUAL "ALL_DATA_STRUCTURES")
      foreach (FULL_CLASS_NAME ${known_data_structures})
        NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
        ADD_DATA_STRUCTURES_TO_BUILD(${FULL_CLASS_NAME})
      endforeach()
    elseif (CURRENT_STATE EQUAL ${STATE_NONE})
      print_error("Syntax error in ADD_TO_PLUGIN: prefix with STEPS, CATEGORIES, ..., Got ${arg}." )
    elseif (CURRENT_STATE EQUAL ${STATE_STEPS})
      ADD_STEP_SOURCES_TO_BUILD(${arg})
    elseif (CURRENT_STATE EQUAL ${STATE_KERNELS})
      ADD_KERNEL_SOURCES_TO_BUILD(${arg})
    elseif (CURRENT_STATE EQUAL ${STATE_DATA_STRUCTURES})
      ADD_DATA_STRUCTURES_TO_BUILD(${arg})
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
