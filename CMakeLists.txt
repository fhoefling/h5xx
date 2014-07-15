cmake_minimum_required(VERSION 2.8)

set(CMAKE_USER_MAKE_RULES_OVERRIDE "cmake/platform.cmake")

project(H5XX CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# The FindBoost CMake module prefers multi-threaded libraries (filenames with
# postfix "-mt") over non-multi-threaded libraries. On Redhat or SuSE with
# installed system Boost libraries, this causes the system libraries (with
# "-mt") to override the custom-compiled libraries (without "-mt").
if(NOT DEFINED Boost_USE_MULTITHREADED)
  set(Boost_USE_MULTITHREADED FALSE)
endif(NOT DEFINED Boost_USE_MULTITHREADED)

find_package(Boost 1.40.0 QUIET REQUIRED COMPONENTS unit_test_framework)
find_package(HDF5 QUIET REQUIRED)

include_directories("${HDF5_INCLUDE_DIR}")
include_directories("${Boost_INCLUDE_DIRS}")
include_directories("${CMAKE_SOURCE_DIR}")

enable_testing()
include(CTest)
add_subdirectory(test)
