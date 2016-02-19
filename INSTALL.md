# Installation of h5xx

h5xx is a C++ template library and does not require compilation. To use h5xx in
your code, include `h5xx.hpp` in your source file and add the h5xx directory to
the compiler's include search path (using `-I`). In addition, make sure that the
HDF5 and Boost headers and the HDF5 libraries are passed to the compiler/linker.

To compile the unit tests and examples, the script `h5xx_build.sh` can be used
as a starting point. CMAKE is required to build the executables.  The CMAKE
configuration files give an example of how to integrate h5xx, HDF5 and Boost
into a code project.
