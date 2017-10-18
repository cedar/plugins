find_package(PCL 1.7 REQUIRED)
include_directories(${PCL_INCLUDE_DIRS})
link_directories(${PCL_LIBARY_DIRS})
set(PCL_LIBS ${PCL_LIBRARIES})

add_definitions(${PCL_DEFINITIONS})
set(PCL_BUILD_TYPE Release)

