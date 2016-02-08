#!/bin/bash

# --- h5xx configure/build script example ---

# Uncomment the following line when you edit
# this script and adapt it to your local environment.
#echo "h5xx build example.  Please adapt this script first.  Exiting." && exit 1

# --- actions
BUILD="yes"
TEST="yes"
#DOXYGEN="yes"

# --- configuration
MPI="yes"
# --- build directory
H5XX_BUILD_PREFIX=$HOME/h5xx
# --- source code location
export H5XX_ROOT=`pwd`
# --- Boost library installation directory
export BOOST_ROOT=/opt/apps/boost/1.58
# --- HDF5 library installation directory
export HDF5_ROOT=/opt/apps/hdf5/1.8.14
if [ x"$MPI" == x"yes" ]
then
  export HDF5_ROOT=${HDF5_ROOT}-mpi
fi

# --- configure, (optionally) build tests and examples, (optionally) run tests
mkdir -p $H5XX_BUILD_PREFIX && cd $H5XX_BUILD_PREFIX && \
cmake -DMPI=${MPI} $H5XX_ROOT && \
if [ x"$BUILD" == x"yes" ]
then
  make -j `nproc`
  if [ x"$TEST" == x"yes" ]
  then
    make test
  fi
fi

# --- create doxygen documentation
if [ x"$DOXYGEN" == x"yes" ]
then
  export H5XX_DOC_PREFIX=${H5XX_BUILD_PREFIX}/doc
  mkdir -p $H5XX_DOC_PREFIX && \
  doxygen ${H5XX_ROOT}/doxygen/h5xx.doxyfile >/dev/null 2>&1 && \
  echo "--- Created doxygen documentation at ${H5XX_DOC_PREFIX}/index.html"
fi
