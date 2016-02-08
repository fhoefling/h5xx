# h5xx -- a template based C++ wrapper for the HDF5 library


## Introduction


## Requirements

* A C++ compiler that supports at least C++98 is necessary.  h5xx was developed
and tested using g++ on Linux most of the time.  In addition, clang and the
Intel C++ compiler should work.
* The cmake build system is required to compile the example and test codes that
are packaged with h5xx.
* HDF5: h5xx requires an installation of the HDF5 library.  The (deprecated)
HDF5 C++ bindings are not required.  HDF5 may be built with MPI to support
parallel IO.
* Boost: h5xx supports the Boost multidimensional array datatype.  Moreover,
h5xx uses the Boost `enable_if` set of templates to control the creation of
SFINAE (substitution-failure-is-not-an-error) conditions.

Please see the file INSTALL.md for further details.
