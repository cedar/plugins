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

# Normalizes a class name so it can be used as part of the name of a cmake variable.
# Throughout the project, this is used to emulate (hash)map-like functionality.
macro(NORMALIZE_CLASS_NAME FULL_CLASS_NAME)
  string(REGEX REPLACE "::" "__" NORMALIZED_CLASS_NAME ${FULL_CLASS_NAME})
endmacro(NORMALIZE_CLASS_NAME)

macro(EXTRACT_CLASS_NAME FULL_CLASS_NAME)
  string(REGEX REPLACE ".*::([[A-Za-z0-9_]*)$" "\\1" CLASS_NAME ${FULL_CLASS_NAME})
endmacro(EXTRACT_CLASS_NAME)

#
# adds the source files, headers, moc headers and required libraries for a given class to the build
#
macro(ADD_CLASS_TO_BUILD_COMMON FULL_CLASS_NAME)
  NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
  set(SOURCE_FILES "${source_files_${NORMALIZED_CLASS_NAME}}")
  list(APPEND PLUGIN_SOURCE_FILES ${SOURCE_FILES})
  
  list(LENGTH "moc_headers_${NORMALIZED_CLASS_NAME}" list_length)
  if (list_length GREATER 0)
    qt_wrap_cpp(moc_sources ${moc_headers_${NORMALIZED_CLASS_NAME}})
    list(APPEND PLUGIN_SOURCE_FILES ${moc_sources})
  endif()
  
  if (REQUIRES_LIBRARIES_${NORMALIZED_CLASS_NAME})
    list(APPEND PLUGIN_REQUIRES_LIBRARIES ${REQUIRES_LIBRARIES_${NORMALIZED_CLASS_NAME}})
  endif()
  
  if (REQUIRES_DATA_STRUCTURES_${NORMALIZED_CLASS_NAME})
    foreach(data_structure_full_class_name ${REQUIRES_DATA_STRUCTURES_${NORMALIZED_CLASS_NAME}})
      ADD_DATA_STRUCTURES_TO_BUILD(${data_structure_full_class_name})
    endforeach()
  endif()
endmacro(ADD_CLASS_TO_BUILD_COMMON)

#
# macro ADD_STEP_SOURCES_TO_BUILD
#
macro(ADD_STEP_SOURCES_TO_BUILD FULL_CLASS_NAME)
  ADD_CLASS_TO_BUILD_COMMON(${FULL_CLASS_NAME})
  NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
  
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
  
  set(deprecated_names "${DEPRECATED_NAMES_${NORMALIZED_CLASS_NAME}}")
  if (deprecated_names)
    foreach (deprecated_name ${deprecated_names})
      string(REPLACE "::" "." deprecated_name ${deprecated_names})
      set(decl "${decl}    declaration->deprecatedName(\"${deprecated_name}\");\n")
    endforeach()
  endif()
  
  set(decl "${decl}    plugin->add(declaration);\n")
  set(decl "${decl}  }\n")
  set(PLUGIN_DECLARATIONS "${PLUGIN_DECLARATIONS} ${decl}")
    
  foreach (header ${header_files_${NORMALIZED_CLASS_NAME}})
    set(PLUGIN_INCLUDE_FILES "${PLUGIN_INCLUDE_FILES} \#include \"${header}\"\n")
  endforeach()
endmacro(ADD_STEP_SOURCES_TO_BUILD)

#
# macro ADD_KERNEL_SOURCES_TO_BUILD
#
macro(ADD_KERNEL_SOURCES_TO_BUILD FULL_CLASS_NAME)
  ADD_CLASS_TO_BUILD_COMMON(${FULL_CLASS_NAME})
  NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
  set(decl "   {\n")
  set(decl "${decl}      cedar::aux::kernel::FactoryManagerSingleton::getInstance()->registerType<${FULL_CLASS_NAME}Ptr>();\n")

  set(deprecated_names "${DEPRECATED_NAMES_${NORMALIZED_CLASS_NAME}}")
  if (deprecated_names)
    foreach (deprecated_name ${deprecated_names})
      string(REPLACE "::" "." deprecated_name ${deprecated_names})
      set(decl "${decl}      cedar::aux::kernel::FactoryManagerSingleton::getInstance()->addDeprecatedName<${FULL_CLASS_NAME}Ptr>(\"${deprecated_name}\");\n")
    endforeach()
  endif()
  
  set(decl "${decl}    }\n")
  set(NONPLUGIN_DECLARATIONS "${NONPLUGIN_DECLARATIONS} ${decl}")
    
  foreach (header ${header_files_${NORMALIZED_CLASS_NAME}})
    set(NONPLUGIN_INCLUDE_FILES "${NONPLUGIN_INCLUDE_FILES} \#include \"${header}\"\n")
  endforeach()
endmacro(ADD_KERNEL_SOURCES_TO_BUILD)

macro(ADD_DATA_STRUCTURES_TO_BUILD FULL_CLASS_NAME)
  ADD_CLASS_TO_BUILD_COMMON(${FULL_CLASS_NAME})
endmacro(ADD_DATA_STRUCTURES_TO_BUILD)

macro(GENERATE_DESCRIPTION_LIST_ENTRY FULL_CLASS_NAME)
  NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
  EXTRACT_CLASS_NAME(${FULL_CLASS_NAME})
  set(description ${DESCRIPTION_${NORMALIZED_CLASS_NAME}})
  if (NOT description)
    set(description "no description.")
  endif()
  
  set(DESCRIPTION_LIST_TEXT "| *${CLASS_NAME}* | ${description} |\n")
endmacro(GENERATE_DESCRIPTION_LIST_ENTRY)

# Adds all files for a class to the appropriate lists, i.e., the list of headers and cpp files per class.
macro(DECLARE_CLASS_FILES FULL_CLASS_NAME)
  # extract the class name, without the namespace
  EXTRACT_CLASS_NAME(${FULL_CLASS_NAME})
  # normalize the class name so it can be used to declare variables
  NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
  
  # TODO check if this file exists; if not, don't add it
  list(APPEND "source_files_${NORMALIZED_CLASS_NAME}" "${CMAKE_CURRENT_LIST_DIR}/${CLASS_NAME}.cpp")
  # these headers are used to add them to the generated plugin.cpp file
  list(APPEND "header_files_${NORMALIZED_CLASS_NAME}" "${CMAKE_CURRENT_LIST_DIR}/${CLASS_NAME}.h")
  
  if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/${CLASS_NAME}.svg")
    list(APPEND "icon_${NORMALIZED_CLASS_NAME}" "${CLASS_NAME}.svg")
    list(APPEND "icon_path_${NORMALIZED_CLASS_NAME}" "${CMAKE_CURRENT_LIST_DIR}/${CLASS_NAME}.svg")
  endif()
endmacro(DECLARE_CLASS_FILES)

macro(DECLARE_MOC_HEADERS FULL_CLASS_NAME)
  # extract the class name, without the namespace
  EXTRACT_CLASS_NAME(${FULL_CLASS_NAME})
  # normalize the class name so it can be used to declare variables
  NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
  
  list(APPEND "moc_headers_${NORMALIZED_CLASS_NAME}" "${CMAKE_CURRENT_LIST_DIR}/${CLASS_NAME}.h")
endmacro(DECLARE_MOC_HEADERS)

macro(DECLARE_CLASS_DESCRIPTION FULL_CLASS_NAME DESCRIPTION)
  # normalize the class name so it can be used to declare variables
  NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
  
  set("DESCRIPTION_${NORMALIZED_CLASS_NAME}" ${DESCRIPTION})
endmacro(DECLARE_CLASS_DESCRIPTION)

macro(DECLARE_DEPRECATED_NAME FULL_CLASS_NAME DEPRECATED_NAME)
  # normalize the class name so it can be used to declare variables
  NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
  
  list(APPEND "DEPRECATED_NAMES_${NORMALIZED_CLASS_NAME}" ${DEPRECATED_NAME})
endmacro(DECLARE_DEPRECATED_NAME)

macro(DECLARE_REQUIRED_LIBRARY FULL_CLASS_NAME REQUIRED_LIBRARY)
  # normalize the class name so it can be used to declare variables
  NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
  
  list(APPEND "REQUIRES_LIBRARIES_${NORMALIZED_CLASS_NAME}" ${REQUIRED_LIBRARY})
endmacro(DECLARE_REQUIRED_LIBRARY)

macro(DECLARE_DATA_STRUCTURE_DEPENDENCY FULL_CLASS_NAME DATA_STRUCTURE_FULL_CLASS_NAME)
  # normalize the class name so it can be used to declare variables
  NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
  
  list(APPEND "REQUIRES_DATA_STRUCTURES_${NORMALIZED_CLASS_NAME}" ${DATA_STRUCTURE_FULL_CLASS_NAME})
endmacro(DECLARE_DATA_STRUCTURE_DEPENDENCY)
