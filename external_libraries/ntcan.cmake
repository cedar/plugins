# This is the file that takes care of finding ntcan

# Find NTCAN and create the following output:
# NTCAN_FOUND - whether naoqi was found or not
# NTCAN_INCLUDE_DIRS - the naoqi include directories
# NTCAN_LIBS - naoqi libraries

# find include dir in set of paths
find_path(NTCAN_INCLUDE
  NAMES ntcan.h
  PATHS ${EXTERNAL_LIBRARY_INCLUDE_PATHS}
)
# find library in set of paths
set(LIBS ntcan)

foreach(LIB ${LIBS})
  find_library(${LIB}-FOUND_LIB NAMES ${LIB} PATHS ${EXTERNAL_LIBRARY_PATHS})

  if (${LIB}-FOUND_LIB)
    list(APPEND NTCAN_LIBS ${${LIB}-FOUND_LIB})
  endif (${LIB}-FOUND_LIB)

endforeach(LIB)

# now check if anything is missing
if(NTCAN_INCLUDE AND NTCAN_LIBS)
  message(STATUS " ${LIB} found")
  set(NTCAN_FOUND true)
else(NTCAN_INCLUDE AND NTCAN_LIBS)
  message(STATUS " ${LIB} NOT FOUND")
  set(NTCAN_FOUND false)
endif(NTCAN_INCLUDE AND NTCAN_LIBS)
