# This is the file that takes care of finding the openni package from pcl
# This file is experimental!!! 
# TODO proper doc

# Find NAOQI and create the following output:
# OPENNI_FOUND - whether openni was found or not
# OPENNI_INCLUDE_DIRS - the openni include directories
# OPENNI_LIBS - openni libraries

# find include dir in set of paths
find_path(OPENNI_INCLUDE
  NAMES XnVersion.h
  PATHS ${EXTERNAL_LIBRARY_INCLUDE_PATHS} /usr/include/ni
)
# find library in set of paths
set(LIBS XnCore)

message(STATUS "Searching for the following OpenNI libraries:")
foreach(LIB ${LIBS})
  find_library(${LIB}-FOUND_LIB NAMES ${LIB} PATHS ${EXTERNAL_LIBRARY_PATHS})

  if (${LIB}-FOUND_LIB)
    message(STATUS "  ${LIB} - found.")
    list(APPEND OPENNI_LIBS ${${LIB}-FOUND_LIB})
  else (${LIB}-FOUND_LIB)
    message(STATUS " ${LIB} - not found.")
  endif (${LIB}-FOUND_LIB)

endforeach(LIB)

# now check if anything is missing
if(OPENNI_INCLUDE AND OPENNI_LIBS)
  set(OPENNI_FOUND true)
else(OPENNI_INCLUDE AND OPENNI_LIBS)
  set(OPENNI_FOUND false)
endif(OPENNI_INCLUDE AND OPENNI_LIBS)
