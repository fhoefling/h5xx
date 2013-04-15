cmake_minimum_required(VERSION 2.8)

project(EXAMPLE CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(HDF5 QUIET REQUIRED)
find_package(H5XX QUIET REQUIRED)

# The FindBoost CMake module prefers multi-threaded libraries (filenames with
# postfix "-mt") over non-multi-threaded libraries. On Redhat or SuSE with
# installed system Boost libraries, this causes the system libraries (with
# "-mt") to override the custom-compiled libraries (without "-mt").
if(NOT DEFINED Boost_USE_MULTITHREADED)
  set(Boost_USE_MULTITHREADED FALSE)
endif(NOT DEFINED Boost_USE_MULTITHREADED)

find_package(Boost 1.40.0 QUIET REQUIRED)

include_directories("${HDF5_INCLUDE_DIR}")
include_directories("${H5XX_INCLUDE_DIR}")
include_directories("${CMAKE_SOURCE_DIR}")

set(CMAKE_BUILD_TYPE_INIT "Release")

add_executable(example
    example.cpp
)

target_link_libraries(example
    ${HDF5_CPP_LIBRARY}
    ${HDF5_LIBRARY}
)
