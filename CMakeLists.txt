cmake_minimum_required (VERSION 2.6)
project (echo)

set(MLAB_LIBRARIES_ROOT ${PROJECT_SOURCE_DIR}/third_party/m-lab.libraries/trunk)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

add_subdirectory(${MLAB_LIBRARIES_ROOT})
add_subdirectory(src)
