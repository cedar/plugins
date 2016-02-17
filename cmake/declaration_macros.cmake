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
  
  # parse optional arguments
  SET_DEFAULT_CLASS_DECLARATION_ARGUMENTS(STEP)
  SET_CATEGORY_DEFAULT_ARGUMENTS(STEP)
  SET_DEPRECATED_NAME_DEFAULT_ARGUMENTS(STEP)
  
  PARSE_DEFAULT_ARGUMENTS(STEP ${ARGN})
  PROCESS_DEFAULT_CLASS_DECLARATION_ARGUMENTS(STEP ${FULL_CLASS_NAME})
  PROCESS_DEPRECATED_NAME_DEFAULT_ARGUMENTS(STEP ${FULL_CLASS_NAME})
  PROCESS_CATEGORY_DEFAULT_ARGUMENTS(STEP ${FULL_CLASS_NAME})
endmacro(DECLARE_STEP)

macro(DECLARE_STEP_BASE_CLASS FULL_CLASS_NAME)
  # extract the class name, without the namespace
  EXTRACT_CLASS_NAME(${FULL_CLASS_NAME})
  # normalize the class name so it can be used to declare variables
  NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
  
  # register the step with the list of steps
  list(APPEND known_step_base_classes ${FULL_CLASS_NAME})
  
  # append the auto-determined cpp and h file for the class
  DECLARE_CLASS_FILES(${FULL_CLASS_NAME})
  
  # parse optional arguments
  SET_DEFAULT_CLASS_DECLARATION_ARGUMENTS(STEP_BASE)
  PARSE_DEFAULT_ARGUMENTS(STEP_BASE ${ARGN})
  PROCESS_DEFAULT_CLASS_DECLARATION_ARGUMENTS(STEP_BASE ${FULL_CLASS_NAME})
endmacro(DECLARE_STEP_BASE_CLASS)

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
  
  SET_DEFAULT_CLASS_DECLARATION_ARGUMENTS(KERNEL)
  PARSE_DEFAULT_ARGUMENTS(KERNEL ${ARGN})
  PROCESS_DEFAULT_CLASS_DECLARATION_ARGUMENTS(KERNEL ${FULL_CLASS_NAME})
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
    
  SET_DEFAULT_CLASS_DECLARATION_ARGUMENTS(DATA_STRUCTURE)
  SET_DEPRECATED_NAME_DEFAULT_ARGUMENTS(DATA_STRUCTURE)
  PARSE_DEFAULT_ARGUMENTS(DATA_STRUCTURE ${ARGN})
  PROCESS_DEFAULT_CLASS_DECLARATION_ARGUMENTS(DATA_STRUCTURE ${FULL_CLASS_NAME})
  PROCESS_DEPRECATED_NAME_DEFAULT_ARGUMENTS(DATA_STRUCTURE ${FULL_CLASS_NAME})
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
  # TODO rewrite this using cmake_parse_arguments
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
        ADD_STEP_TO_BUILD(${FULL_CLASS_NAME})
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
      ADD_STEP_TO_BUILD(${arg})
    elseif (CURRENT_STATE EQUAL ${STATE_KERNELS})
      ADD_KERNEL_SOURCES_TO_BUILD(${arg})
    elseif (CURRENT_STATE EQUAL ${STATE_DATA_STRUCTURES})
      ADD_DATA_STRUCTURES_TO_BUILD(${arg})
    elseif (CURRENT_STATE EQUAL ${STATE_CATEGORIES})
      foreach (FULL_CLASS_NAME ${known_steps})
        NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
        set(CATEGORY ${CATEGORY_${NORMALIZED_CLASS_NAME}})
        if (${CATEGORY} STREQUAL arg)
          ADD_STEP_TO_BUILD(${FULL_CLASS_NAME})
        endif()
      endforeach()
    endif()
  endforeach()
endmacro(ADD_TO_PLUGIN)
