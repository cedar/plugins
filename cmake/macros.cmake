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

#
# Adds defaults to several lists to be used with cmake_parse_arguments.
#
macro(SET_DEFAULT_CLASS_DECLARATION_ARGUMENTS PREFIX)
  list(APPEND "${PREFIX}_OPTIONS" "MOC")
  list(APPEND "${PREFIX}_ONE_VALUE_OPTIONS" "DESCRIPTION")
  list(APPEND "${PREFIX}_ONE_VALUE_OPTIONS" "MAINTAINER")
  list(APPEND "${PREFIX}_MULTI_VALUE_OPTIONS" "REQUIRES_DATA_STRUCTURE")
  list(APPEND "${PREFIX}_MULTI_VALUE_OPTIONS" "REQUIRES_DATA_STRUCTURES")
  list(APPEND "${PREFIX}_MULTI_VALUE_OPTIONS" "REQUIRES_STEP_BASE_CLASS")
  list(APPEND "${PREFIX}_MULTI_VALUE_OPTIONS" "REQUIRES_STEP_BASE_CLASSES")
  list(APPEND "${PREFIX}_MULTI_VALUE_OPTIONS" "REQUIRES_STEP")
  list(APPEND "${PREFIX}_MULTI_VALUE_OPTIONS" "REQUIRES_STEPS")
  list(APPEND "${PREFIX}_MULTI_VALUE_OPTIONS" "REQUIRES_LIBRARIES")
  list(APPEND "${PREFIX}_MULTI_VALUE_OPTIONS" "REQUIRES_LIBRARY")
endmacro()

#
# Adds defaults to several lists to be used with cmake_parse_arguments.
#
macro(SET_DEPRECATED_NAME_DEFAULT_ARGUMENTS PREFIX)
  list(APPEND "${PREFIX}_MULTI_VALUE_OPTIONS" "DEPRECATED_NAME")
  list(APPEND "${PREFIX}_MULTI_VALUE_OPTIONS" "DEPRECATED_NAMES")
endmacro()

#
# Adds defaults to several lists to be used with cmake_parse_arguments.
#
macro(SET_DEPRECATION_DEFAULT_ARGUMENTS PREFIX)
  list(APPEND "${PREFIX}_ONE_VALUE_OPTIONS" "DEPRECATED")
endmacro()


#
#
macro(SET_CATEGORY_DEFAULT_ARGUMENTS PREFIX)
  list(APPEND "${PREFIX}_ONE_VALUE_OPTIONS" "CATEGORY")
endmacro()

#
#
macro(PROCESS_CATEGORY_DEFAULT_ARGUMENTS PREFIX FULL_CLASS_NAME)
  NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
  if (${PREFIX}_CATEGORY)
    set("CATEGORY_${NORMALIZED_CLASS_NAME}" ${${PREFIX}_CATEGORY})
    list(APPEND CATEGORIES ${${PREFIX}_CATEGORY})
  endif()
endmacro()

#
# Processes parsed default calss arguments parsed with SET_DEFAULT_CLASS_DECLARATION_ARGUMENTS and PARSE_DEFAULT_ARGUMENTS PREFIX.
#
macro(PROCESS_DEPRECATED_NAME_DEFAULT_ARGUMENTS PREFIX FULL_CLASS_NAME)
  if (STEP_DEPRECATED_NAME)
    foreach (deprecated_name ${${PREFIX}_DEPRECATED_NAME})
      DECLARE_DEPRECATED_NAME(${FULL_CLASS_NAME} ${deprecated_name})
    endforeach()
  endif()
  
  if (STEP_DEPRECATED_NAMES)
    foreach (deprecated_name ${${PREFIX}_DEPRECATED_NAMES})
      DECLARE_DEPRECATED_NAME(${FULL_CLASS_NAME} ${deprecated_name})
    endforeach()
  endif()
endmacro()

#
# Processes parsed default calss arguments parsed with SET_DEFAULT_CLASS_DECLARATION_ARGUMENTS and PARSE_DEFAULT_ARGUMENTS PREFIX.
#
macro(PROCESS_DEPRECATION_DEFAULT_ARGUMENTS PREFIX FULL_CLASS_NAME)
  if (STEP_DEPRECATED)
    DECLARE_DEPRECATION(${FULL_CLASS_NAME} ${STEP_DEPRECATED})
  endif()
endmacro()

#
# Processes parsed default calss arguments parsed with SET_DEFAULT_CLASS_DECLARATION_ARGUMENTS and PARSE_DEFAULT_ARGUMENTS PREFIX.
#
macro(PROCESS_DEFAULT_CLASS_DECLARATION_ARGUMENTS PREFIX FULL_CLASS_NAME)
  NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
  
  if (${PREFIX}_MOC)
    DECLARE_MOC_HEADERS(${FULL_CLASS_NAME})
  endif()
  
  if (${PREFIX}_DESCRIPTION)
    DECLARE_CLASS_DESCRIPTION(${FULL_CLASS_NAME} ${${PREFIX}_DESCRIPTION})
  endif()
  
  if (${PREFIX}_MAINTAINER)
    set("MAINTAINER_${NORMALIZED_CLASS_NAME}" ${${PREFIX}_MAINTAINER})
  endif()
  
  if (${PREFIX}_REQUIRES_DATA_STRUCTURES)
    foreach (data_structure_full_class_name ${${PREFIX}_REQUIRES_DATA_STRUCTURES})
      DECLARE_DATA_STRUCTURE_DEPENDENCY(${FULL_CLASS_NAME} ${data_structure_full_class_name})
    endforeach()
  endif()
  
  if (${PREFIX}_REQUIRES_DATA_STRUCTURE)
    foreach (data_structure_full_class_name ${${PREFIX}_REQUIRES_DATA_STRUCTURE})
      DECLARE_DATA_STRUCTURE_DEPENDENCY(${FULL_CLASS_NAME} ${data_structure_full_class_name})
    endforeach()
  endif()
  
  if (${PREFIX}_REQUIRES_LIBRARIES)
    foreach(library ${${PREFIX}_REQUIRES_LIBRARIES})
      DECLARE_REQUIRED_LIBRARY(${FULL_CLASS_NAME} ${library})
    endforeach()
  endif()
    
  if (${PREFIX}_REQUIRES_LIBRARY)
    foreach(library ${${PREFIX}_REQUIRES_LIBRARY})
      DECLARE_REQUIRED_LIBRARY(${FULL_CLASS_NAME} ${library})
    endforeach()
  endif()
  
  if (${PREFIX}_REQUIRES_STEP_BASE_CLASS)
    foreach(class ${${PREFIX}_REQUIRES_STEP_BASE_CLASS})
      DECLARE_STEP_BASE_CLASS_DEPENDENCY(${FULL_CLASS_NAME} ${class})
    endforeach()
  endif()
  
  if (${PREFIX}_REQUIRES_STEP_BASE_CLASSES)
    foreach(class ${${PREFIX}_REQUIRES_STEP_BASE_CLASSES})
      DECLARE_STEP_BASE_CLASS_DEPENDENCY(${FULL_CLASS_NAME} ${class})
    endforeach()
  endif()
  
  if (${PREFIX}_REQUIRES_STEP)
    foreach(step ${${PREFIX}_REQUIRES_STEP})
      DECLARE_STEP_DEPENDENCY(${FULL_CLASS_NAME} ${step})
    endforeach()
  endif()
  
  if (${PREFIX}_REQUIRES_STEPS)
    foreach(class ${${PREFIX}_REQUIRES_STEPS})
      DECLARE_STEP_DEPENDENCY(${FULL_CLASS_NAME} ${step})
    endforeach()
  endif()
  
  string(TOLOWER ${PREFIX} prefix_lowercase)
  CHECK_DESCRIPTION(${FULL_CLASS_NAME} ${NORMALIZED_CLASS_NAME} "${prefix_lowercase}")
endmacro()

#
# Parses a list of arguments using lists created with SET_DEFAULT_*_DECLARATION_OPTIONS
#
macro(PARSE_DEFAULT_ARGUMENTS PREFIX)
  cmake_parse_arguments(${PREFIX} "${${PREFIX}_OPTIONS}" "${${PREFIX}_ONE_VALUE_OPTIONS}" "${${PREFIX}_MULTI_VALUE_OPTIONS}" ${ARGN})
endmacro()


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

# remember that a class is already being built
macro(MARK_CLASS_IN_BUILD FULL_CLASS_NAME)
  list(APPEND CLASSES_IN_BUILD ${FULL_CLASS_NAME})
endmacro()

# check if a class is already in the build
macro(IS_CLASS_IN_BUILD FULL_CLASS_NAME PREFIX)
  list(FIND CLASSES_IN_BUILD ${FULL_CLASS_NAME} class_in_build)
  if (class_in_build EQUAL -1)
    set("${PREFIX}_IN_BUILD" FALSE)
  else()
    set("${PREFIX}_IN_BUILD" TRUE)
  endif()
endmacro()

#
# adds the source files, headers, moc headers and required libraries for a given class to the build
#
macro(ADD_CLASS_TO_BUILD_COMMON FULL_CLASS_NAME)
  MARK_CLASS_IN_BUILD(${FULL_CLASS_NAME})
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
  
  if (REQUIRES_STEP_BASE_CLASS_${NORMALIZED_CLASS_NAME})
    foreach(base_class_full_name ${REQUIRES_STEP_BASE_CLASS_${NORMALIZED_CLASS_NAME}})
      ADD_STEP_BASE_CLASS_TO_BUILD(${base_class_full_name})
    endforeach()
  endif()
  
  # we need to normalize the class name again, because the calls to other macros above might have overridden it...
  NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
  if (REQUIRES_STEP_${NORMALIZED_CLASS_NAME})
    foreach(step_full_class_name ${REQUIRES_STEP_${NORMALIZED_CLASS_NAME}})
      ADD_STEP_TO_BUILD(${step_full_class_name})
    endforeach()
  endif()
endmacro(ADD_CLASS_TO_BUILD_COMMON)

#
# macro ADD_STEP_SOURCES_TO_BUILD
#
macro(ADD_STEP_TO_BUILD FULL_CLASS_NAME)
  IS_CLASS_IN_BUILD(${FULL_CLASS_NAME} STEP)
  if (NOT STEP_IN_BUILD)
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
    
    # add deprecation, if set ${NORMALIZED_CLASS_NAME}_DEPRECATION
    set (deprecation "${${NORMALIZED_CLASS_NAME}_DEPRECATION}")
    if (deprecation)
      set(decl "${decl}    declaration->deprecate(\"${deprecation}\");\n")
    endif()
    
    set(decl "${decl}    plugin->add(declaration);\n")
    set(decl "${decl}  }\n")
    set(PLUGIN_DECLARATIONS "${PLUGIN_DECLARATIONS} ${decl}")
      
    foreach (header ${header_files_${NORMALIZED_CLASS_NAME}})
      set(PLUGIN_INCLUDE_FILES "${PLUGIN_INCLUDE_FILES} \#include \"${header}\"\n")
    endforeach()
  endif (NOT STEP_IN_BUILD)
endmacro(ADD_STEP_TO_BUILD)

#
# macro ADD_KERNEL_SOURCES_TO_BUILD
#
macro(ADD_KERNEL_SOURCES_TO_BUILD FULL_CLASS_NAME)
  IS_CLASS_IN_BUILD(${FULL_CLASS_NAME} KERNEL)
  if (NOT KERNEL_IN_BUILD)
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
  endif (NOT KERNEL_IN_BUILD)
endmacro(ADD_KERNEL_SOURCES_TO_BUILD)

macro(ADD_DATA_STRUCTURES_TO_BUILD FULL_CLASS_NAME)
  IS_CLASS_IN_BUILD(${FULL_CLASS_NAME} DATA_STRUCTURE)
  if (NOT DATA_STRUCTURE_IN_BUILD)
    ADD_CLASS_TO_BUILD_COMMON(${FULL_CLASS_NAME})
  endif (NOT DATA_STRUCTURE_IN_BUILD)
endmacro(ADD_DATA_STRUCTURES_TO_BUILD)

macro(ADD_STEP_BASE_CLASS_TO_BUILD FULL_CLASS_NAME)
  IS_CLASS_IN_BUILD(${FULL_CLASS_NAME} STEP_BASE_CLASS)
  if (NOT STEP_BASE_CLASS_IN_BUILD)
    ADD_CLASS_TO_BUILD_COMMON(${FULL_CLASS_NAME})
  endif (NOT STEP_BASE_CLASS_IN_BUILD)
endmacro()

macro(ADD_PLOT_TO_BUILD FULL_CLASS_NAME)
  IS_CLASS_IN_BUILD(${FULL_CLASS_NAME} PLOT)
  if (NOT PLOT_IN_BUILD)
    ADD_CLASS_TO_BUILD_COMMON(${FULL_CLASS_NAME})
    NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
    
    set(decl " {\n")
    set(decl "${decl}    auto declaration = boost::make_shared<\n")
    set(decl "${decl}              cedar::aux::gui::PlotDeclarationTemplate<${PLOTTED_DATA_${NORMALIZED_CLASS_NAME}}, ${FULL_CLASS_NAME}>\n")
    set(decl "${decl}              >();\n")
    
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
  endif (NOT PLOT_IN_BUILD)
endmacro(ADD_PLOT_TO_BUILD)

# TODO move to macros
macro(ADD_STEP_CATEGORY_TO_PLUGIN CATEGORY_TO_ADD)
  foreach (FULL_CLASS_NAME ${known_steps})
    NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
    set(CATEGORY ${CATEGORY_${NORMALIZED_CLASS_NAME}})
    if (${CATEGORY} STREQUAL ${CATEGORY_TO_ADD})
      ADD_STEP_TO_BUILD(${FULL_CLASS_NAME})
    endif()
  endforeach()
endmacro()

macro(FILL_STRING STR MIN_LENGTH FILLER OUTPUT_VAR)
  set(${OUTPUT_VAR} ${STR})
  string(LENGTH ${OUTPUT_VAR} LEN)
  while(LEN LESS ${MIN_LENGTH})
    set(${OUTPUT_VAR} "${${OUTPUT_VAR}}${FILLER}")
    string(LENGTH ${${OUTPUT_VAR}} LEN)
  endwhile()
endmacro(FILL_STRING)

macro(GENERATE_GENERIC_DESCRIPTION_TABLE_ROW LEFT_STRING LEFT_WIDTH RIGHT_STRING RIGHT_WIDTH FILLER SEPARATOR OUTPUT_VAR)
  FILL_STRING("${FILLER}${LEFT_STRING}" ${LEFT_WIDTH} ${FILLER} FILLED_LEFT_STRING)
  FILL_STRING("${FILLER}${RIGHT_STRING}" ${RIGHT_WIDTH} ${FILLER} FILLED_RIGHT_STRING)
  set(${OUTPUT_VAR} "${${OUTPUT_VAR}}${SEPARATOR}${FILLED_LEFT_STRING}${SEPARATOR}${FILLED_RIGHT_STRING}${SEPARATOR}\n")
endmacro()

macro(GENERATE_DESCRIPTION_TABLE_SEPARATOR OUTPUT_VAR)
  GENERATE_GENERIC_DESCRIPTION_TABLE_ROW("" 30 "" 80 "-" "|" ${OUTPUT_VAR})
endmacro()

macro(GENERATE_DESCRIPTION_TABLE_SPAN TEXT OUTPUT_VAR)
  FILL_STRING(" ${TEXT}" 111 " " FILLED_TEXT)
  set(${OUTPUT_VAR} "${${OUTPUT_VAR}}|${FILLED_TEXT}|\n")
endmacro()

macro(GENERATE_DESCRIPTION_TABLE_HEADER LEFT RIGHT OUTPUT_VAR)
  GENERATE_GENERIC_DESCRIPTION_TABLE_ROW(${LEFT} 30 ${RIGHT} 80 " " "|" ${OUTPUT_VAR})
endmacro()

macro(GENERATE_DESCRIPTION_TABLE_ROW LEFT RIGHT OUTPUT_VAR)
  GENERATE_GENERIC_DESCRIPTION_TABLE_ROW(${LEFT} 30 ${RIGHT} 80 " " "|" ${OUTPUT_VAR})
endmacro()

macro(GENERATE_DESCRIPTION_LIST TYPE_STRING LIST_NAME OUTPUT_VAR)
  list(SORT ${LIST_NAME})
  set(${OUTPUT_VAR} "")
  GENERATE_DESCRIPTION_TABLE_HEADER(${TYPE_STRING} "description" ${OUTPUT_VAR})
  GENERATE_DESCRIPTION_TABLE_SEPARATOR(${OUTPUT_VAR})
  foreach (FULL_CLASS_NAME ${${LIST_NAME}})
    GENERATE_DESCRIPTION_LIST_ENTRY(${FULL_CLASS_NAME})
    set(${OUTPUT_VAR} "${${OUTPUT_VAR}}${DESCRIPTION_LIST_TEXT}")
  endforeach()
  set(${OUTPUT_VAR} "${${OUTPUT_VAR}}\n\n")
endmacro()

macro(GENERATE_DESCRIPTION_LIST_ENTRY FULL_CLASS_NAME)
  NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
  EXTRACT_CLASS_NAME(${FULL_CLASS_NAME})
  set(description ${DESCRIPTION_${NORMALIZED_CLASS_NAME}})
  if (NOT description)
    set(description "no description.")
  endif()
  
#  set(DESCRIPTION_LIST_TEXT "| *${CLASS_NAME}* | ${description} |\n")
  set(DESCRIPTION_LIST_TEXT "")
  GENERATE_DESCRIPTION_TABLE_ROW("*${CLASS_NAME}*" ${description} DESCRIPTION_LIST_TEXT)
endmacro(GENERATE_DESCRIPTION_LIST_ENTRY)

# Adds all files for a class to the appropriate lists, i.e., the list of headers and cpp files per class.
macro(DECLARE_CLASS_FILES FULL_CLASS_NAME)
  # extract the class name, without the namespace
  EXTRACT_CLASS_NAME(${FULL_CLASS_NAME})
  # normalize the class name so it can be used to declare variables
  NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
  
  set(CPP_FILE "${CMAKE_CURRENT_LIST_DIR}/${CLASS_NAME}.cpp")
  set(HEADER_FILE "${CMAKE_CURRENT_LIST_DIR}/${CLASS_NAME}.h")
  set(ICON_FILE "${CMAKE_CURRENT_LIST_DIR}/${CLASS_NAME}.svg")
  
  if (EXISTS ${CPP_FILE})
    list(APPEND "source_files_${NORMALIZED_CLASS_NAME}" "${CPP_FILE}")
  endif()
  
  # these headers are used to add them to the generated plugin.cpp file
  if (EXISTS ${HEADER_FILE})
    list(APPEND "header_files_${NORMALIZED_CLASS_NAME}" "${HEADER_FILE}")
  endif()
  
  if (EXISTS "${ICON_FILE}")
    list(APPEND "icon_${NORMALIZED_CLASS_NAME}" "${CLASS_NAME}.svg")
    list(APPEND "icon_path_${NORMALIZED_CLASS_NAME}" "${ICON_FILE}")
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

macro(DECLARE_DEPRECATION FULL_CLASS_NAME DEPRECATION)
  # normalize the class name so it can be used to declare variables
  NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
  
  set("${NORMALIZED_CLASS_NAME}_DEPRECATION" ${DEPRECATION})
endmacro(DECLARE_DEPRECATION)

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

macro(DECLARE_STEP_BASE_CLASS_DEPENDENCY FULL_CLASS_NAME STEP_BASE_FULL_CLASS_NAME)
  # normalize the class name so it can be used to declare variables
  NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})

  list(APPEND "REQUIRES_STEP_BASE_CLASS_${NORMALIZED_CLASS_NAME}" ${STEP_BASE_FULL_CLASS_NAME})
endmacro()

macro(DECLARE_STEP_DEPENDENCY FULL_CLASS_NAME STEP_FULL_CLASS_NAME)
  # normalize the class name so it can be used to declare variables
  NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})

  list(APPEND "REQUIRES_STEP_${NORMALIZED_CLASS_NAME}" ${STEP_FULL_CLASS_NAME})
endmacro()
