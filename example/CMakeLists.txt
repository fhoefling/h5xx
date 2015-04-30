cmake_minimum_required(VERSION 2.8)

project(EXAMPLE CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(Boost 1.40.0 QUIET REQUIRED)
find_package(HDF5 QUIET REQUIRED)
find_package(H5XX QUIET REQUIRED)

include_directories(SYSTEM "${Boost_INCLUDE_DIR}")
include_directories(SYSTEM "${HDF5_INCLUDE_DIR}")
include_directories("${H5XX_INCLUDE_DIR}")

set(CMAKE_BUILD_TYPE "Release")

add_executable(example
    example.cpp
)

target_link_libraries(example
    ${HDF5_CPP_LIBRARY}
    ${HDF5_LIBRARY}
)
