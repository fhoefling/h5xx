cmake_minimum_required(VERSION 2.8)

project(EXAMPLE CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(Boost 1.40.0 QUIET REQUIRED)
find_package(HDF5 QUIET REQUIRED)
find_package(H5XX QUIET REQUIRED)

find_package(MPI QUIET)
if (MPI_FOUND)
   include_directories(${MPI_INCLUDE_PATH})
   link_libraries(${MPI_LIBRARIES})
   add_definitions(-DUSE_MPI)
endif()

include_directories(${Boost_INCLUDE_DIR})
include_directories(${HDF5_INCLUDE_DIR})
include_directories(${H5XX_INCLUDE_DIR})

set(CMAKE_BUILD_TYPE "Release")

# --- non-MPI programs
foreach(module
  attribute
  dataset_multiarray
  slice_multiarray
  slice_array
  slice_std_vector
  slice_python
  )
  add_executable(${module}
    ${module}.cpp
  )
  target_link_libraries(${module}
#    ${HDF5_CPP_LIBRARY}
    ${HDF5_LIBRARY}
    dl
    z
  )
endforeach()

if (MPI_FOUND)
  foreach(module
    slice_mpi
    )
    add_executable(${module}
      ${module}.cpp
    )
    target_link_libraries(${module}
#    ${HDF5_CPP_LIBRARY}
      ${HDF5_LIBRARY}
      dl
      z
    )
  endforeach()
endif()
