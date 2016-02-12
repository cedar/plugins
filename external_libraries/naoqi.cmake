# This is the file that takes care of finding the naoqi
# TODO proper doc

# Find NAOQI and create the following output:
# NAOQI_FOUND - whether naoqi was found or not
# NAOQI_INCLUDE_DIRS - the naoqi include directories
# NAOQI_LIBS - naoqi libraries

# find include dir in set of paths
find_path(NAOQI_INCLUDE
  NAMES almath.i
  PATHS ${EXTERNAL_LIBRARY_INCLUDE_PATHS}
)
# find library in set of paths
set(LIBS alerror alproxies albase motion videoinput sensors alvalue altools almath alcommon)

message(STATUS "Searching for the following NAOqi libraries:")
foreach(LIB ${LIBS})
  find_library(${LIB}-FOUND_LIB NAMES ${LIB} PATHS ${EXTERNAL_LIBRARY_PATHS} ${EXTERNAL_LIBRARY_PATHS}/naoqi)

  if (${LIB}-FOUND_LIB)
    message(STATUS "  ${LIB} - found.")
    list(APPEND NAOQI_LIBS ${${LIB}-FOUND_LIB})
  else (${LIB}-FOUND_LIB)
    message(STATUS " ${LIB} - not found.")
  endif (${LIB}-FOUND_LIB)

endforeach(LIB)

# now check if anything is missing
if(NAOQI_INCLUDE AND NAOQI_LIBS)
  set(NAOQI_FOUND true)
else(NAOQI_INCLUDE AND NAOQI_LIBS)
  set(NAOQI_FOUND false)
endif(NAOQI_INCLUDE AND NAOQI_LIBS)
