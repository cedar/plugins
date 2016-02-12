find_path(VISLAB_TOOLBOX_INCLUDE
  NAMES keypoints/keypoints.h
  PATHS ${EXTERNAL_LIBRARY_INCLUDE_PATHS}
)

# find include dir in set of paths
set(LIBS keypoints)

message(STATUS "Searching for the following vision lab libraries:")
foreach(LIB ${LIBS})
  find_library(${LIB}-FOUND_LIB NAMES ${LIB} PATHS ${EXTERNAL_LIBRARY_PATHS}/${LIB})

  if (${LIB}-FOUND_LIB)
    message(STATUS "  ${LIB} - found.")
    list(APPEND VISLAB_TOOLBOX_LIBS ${${LIB}-FOUND_LIB})
  else (${LIB}-FOUND_LIB)
    message(STATUS " ${LIB} - not found.")
  endif (${LIB}-FOUND_LIB)

endforeach(LIB)


# now check if anything is missing
if(VISLAB_TOOLBOX_INCLUDE AND VISLAB_TOOLBOX_LIBS)
  set(VISLAB_TOOLBOX_FOUND true)
else(VISLAB_TOOLBOX_INCLUDE AND VISLAB_TOOLBOX_LIBS)
  set(VISLAB_TOOLBOX_FOUND false)
endif()
