#!/bin/bash

# --- h5xx configure/build script example (Linux) ---

# Uncomment the following line when you edit
# this script and adapt it to your local environment.
#echo "h5xx build example.  Please adapt this script first.  Exiting." && exit 1

# --- actions
CLEAN="yes"
BUILD="yes"
TEST="no"
#DOXYGEN="yes"

# --- select compiler
export CXX="g++"
# export CXX="clang"
# export CXX="icpc"
# --- parallel build
# NPROC=`nproc`
# --- MPI support, requires an MPI-HDF5 build
MPI="no"
# --- build directory
H5XX_BUILD_PREFIX=${HOME}/h5xx
# --- source code location
export H5XX_ROOT=`pwd`
# --- Boost library installation directory
export BOOST_ROOT=/opt/apps/boost/1.60
# --- HDF5 library installation directory
export HDF5_ROOT=/opt/apps/hdf5/1.8.16
if [ x"$MPI" == x"yes" ]
then
  #export CXX=mpicxx
  export HDF5_ROOT=${HDF5_ROOT}-mpi
fi
# --- generate colored GCC error output (only supported by recent GCC versions)
export GCC_COLORS='error=01;31:warning=01;35:note=01;36:caret=01;32:locus=01:quote=01'


if [ x"$CLEAN" == x"yes" ]
then
  mv $H5XX_BUILD_PREFIX ${H5XX_BUILD_PREFIX}.trash && \
  rm -rf ${H5XX_BUILD_PREFIX}.trash
fi

# --- configure, (optionally) build tests and examples, (optionally) run tests
mkdir -p $H5XX_BUILD_PREFIX && cd $H5XX_BUILD_PREFIX && \
cmake -DMPI=${MPI} $H5XX_ROOT \
      -DCMAKE_COLOR_MAKEFILE:BOOL=ON \
      -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON && \
if [ x"$BUILD" == x"yes" ]
then
  if [ x"$NPROC" != x"" ]
  then
    make -j $NPROC
  else
    make
  fi
  if [ x"$TEST" == x"yes" ]
  then
    make test
  fi
fi


# --- create doxygen documentation (H5XX_ROOT and H5XX_DOC_PREFIX)
if [ x"$DOXYGEN" == x"yes" ]
then
  export H5XX_DOC_PREFIX=${H5XX_BUILD_PREFIX}/doc
  mkdir -p $H5XX_DOC_PREFIX && \
  doxygen ${H5XX_ROOT}/doxygen/h5xx.doxyfile >/dev/null 2>&1 && \
  echo "--- Created doxygen documentation at ${H5XX_DOC_PREFIX}/index.html"
fi
