# - Find H5XX
# Find the C++ template wrapper for the Hierarchical Data Format 5 (HDF5) library
#
# This module defines
#  H5XX_FOUND
#  H5XX_INCLUDE_DIR
#

#=============================================================================
# Copyright 2002-2009 Kitware, Inc.
# Copyright 2013      Felix Höfling
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file COPYING-CMAKE-SCRIPTS for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

find_path(H5XX_INCLUDE_DIR h5xx/h5xx.hpp
  HINTS
  $ENV{H5XX_DIR} $ENV{H5XX_ROOT} $ENV{H5XX_HOME}
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(H5XX DEFAULT_MSG
  H5XX_INCLUDE_DIR
)

mark_as_advanced(
  H5XX_INCLUDE_DIR
)
