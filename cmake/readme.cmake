# generate the welcome text #
file(READ ${CMAKE_SOURCE_DIR}/cmake/readme_src/welcome.md WELCOME_TEXT)

# generate the step list #

list(REMOVE_DUPLICATES CATEGORIES)
list(SORT CATEGORIES)

set(STEP_LIST_TEXT "${STEP_LIST_TEXT}| step     | description |\n")
set(STEP_LIST_TEXT "${STEP_LIST_TEXT}|----------|-------------|\n")
foreach (category ${CATEGORIES})
  set(STEP_LIST_TEXT "${STEP_LIST_TEXT}| **${category}** |\n")
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

# generate the kernel list #
list(SORT known_kernels)

set(KERNEL_LIST_TEXT "${KERNEL_LIST_TEXT}| kernel   | description |\n")
set(KERNEL_LIST_TEXT "${KERNEL_LIST_TEXT}|----------|-------------|\n")
foreach (FULL_CLASS_NAME ${known_kernels})
  GENERATE_DESCRIPTION_LIST_ENTRY(${FULL_CLASS_NAME})
  set(KERNEL_LIST_TEXT "${KERNEL_LIST_TEXT}${DESCRIPTION_LIST_TEXT}")
endforeach()
set(KERNEL_LIST_TEXT "${KERNEL_LIST_TEXT}\n\n")

# generate the data structures list #
list(SORT known_data_structures)

set(DATA_STRUCTURES_LIST_TEXT "${DATA_STRUCTURES_LIST_TEXT}| data structure | description |\n")
set(DATA_STRUCTURES_LIST_TEXT "${DATA_STRUCTURES_LIST_TEXT}|----------------|-------------|\n")
foreach (FULL_CLASS_NAME ${known_data_structures})
  GENERATE_DESCRIPTION_LIST_ENTRY(${FULL_CLASS_NAME})
  set(DATA_STRUCTURES_LIST_TEXT "${DATA_STRUCTURES_LIST_TEXT}${DESCRIPTION_LIST_TEXT}")
endforeach()
set(DATA_STRUCTURES_LIST_TEXT "${DATA_STRUCTURES_LIST_TEXT}\n\n")

# generate the descriptions for external libraries #

file(GLOB_RECURSE ext_descriptions "${CMAKE_SOURCE_DIR}/external_libraries/*.md")
list(SORT ext_descriptions)
foreach (description ${ext_descriptions})
  file(READ ${description} description_md)
  set(EXTERNAL_LIBRARIES_TEXT "${EXTERNAL_LIBRARIES_TEXT}${description_md}")
endforeach()

configure_file(${CMAKE_SOURCE_DIR}/cmake/readme_src/readme.md.in ${CMAKE_SOURCE_DIR}/readme.md)
