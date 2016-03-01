cmake_minimum_required(VERSION 2.8)

set(CMAKE_USER_MAKE_RULES_OVERRIDE "cmake/platform.cmake")

project(H5XX CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

option(MPI "Build with MPI support for parallel IO" OFF)

# The FindBoost CMake module prefers multi-threaded libraries (filenames with
# postfix "-mt") over non-multi-threaded libraries. On Redhat or SuSE with
# installed system Boost libraries, this causes the system libraries (with
# "-mt") to override the custom-compiled libraries (without "-mt").
if(NOT DEFINED Boost_USE_MULTITHREADED)
  set(Boost_USE_MULTITHREADED FALSE)
endif(NOT DEFINED Boost_USE_MULTITHREADED)

find_package(Boost 1.40.0 QUIET REQUIRED COMPONENTS unit_test_framework)
find_package(HDF5 QUIET REQUIRED)

#
# use
# $ cmake -DMPI=ON ...
# to configure with MPI support
#
if (MPI)
   find_package(MPI)
endif()
if (MPI_FOUND)
   include_directories(${MPI_INCLUDE_PATH})
   link_libraries(${MPI_LIBRARIES})
   add_definitions(-DH5XX_USE_MPI -DMPICH_IGNORE_CXX_SEEK)
endif()

include_directories(SYSTEM ${HDF5_INCLUDE_DIR})
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR})

enable_testing()
include(CTest)
add_subdirectory(test)

set(H5XX_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/h5xx")
add_subdirectory(example)
