# generate the welcome text #
file(READ ${CMAKE_SOURCE_DIR}/cmake/readme_src/welcome.md WELCOME_TEXT)

# generate the step list #

list(REMOVE_DUPLICATES CATEGORIES)
list(SORT CATEGORIES)

GENERATE_DESCRIPTION_TABLE_HEADER(step description STEP_LIST_TEXT)
GENERATE_DESCRIPTION_TABLE_SEPARATOR(STEP_LIST_TEXT)
foreach (category ${CATEGORIES})
  GENERATE_DESCRIPTION_TABLE_SPAN("**${category}**" STEP_LIST_TEXT)
  list(SORT known_steps)
  foreach (FULL_CLASS_NAME ${known_steps})
    NORMALIZE_CLASS_NAME(${FULL_CLASS_NAME})
    set(step_category ${CATEGORY_${NORMALIZED_CLASS_NAME}})
    if (category STREQUAL step_category)
      GENERATE_DESCRIPTION_LIST_ENTRY(${FULL_CLASS_NAME})
      set(STEP_LIST_TEXT "${STEP_LIST_TEXT}${DESCRIPTION_LIST_TEXT}")
    endif()
  endforeach()
endforeach()
set(STEP_LIST_TEXT "${STEP_LIST_TEXT}\n\n")

# generate the group template list #
GENERATE_DESCRIPTION_LIST("group template" known_group_templates GROUP_TEMPLATE_LIST_TEXT)

# generate the device list #
#GENERATE_DESCRIPTION_LIST("device" known_devices DEVICE_LIST_TEXT)

# generate the kernel list #
GENERATE_DESCRIPTION_LIST("kernel" known_kernels KERNEL_LIST_TEXT)

# generate the data structures list #
GENERATE_DESCRIPTION_LIST("data structure" known_data_structures DATA_STRUCTURES_LIST_TEXT)

# generate the data structures list #
GENERATE_DESCRIPTION_LIST("plot" known_plots PLOT_LIST_TEXT)

# generate the descriptions for external libraries #

file(GLOB_RECURSE ext_descriptions "${CMAKE_SOURCE_DIR}/external_libraries/*.md")
list(SORT ext_descriptions)
foreach (description ${ext_descriptions})
  file(READ ${description} description_md)
  set(EXTERNAL_LIBRARIES_TEXT "${EXTERNAL_LIBRARIES_TEXT}${description_md}")
endforeach()

configure_file(${CMAKE_SOURCE_DIR}/cmake/readme_src/readme.md.in ${CMAKE_SOURCE_DIR}/readme.md)
