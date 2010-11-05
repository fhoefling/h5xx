/*
 * Copyright © 2010  Peter Colberg
 *
 * This file is part of HALMD.
 *
 * HALMD is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef H5XX_COMPAT_HPP
#define H5XX_COMPAT_HPP

//
// h5xx wrapper supports the following HDF5 library versions:
//
//  - HDF5 1.8.x compiled using --disable-deprecate-symbols
//  - HDF5 1.8.x compiled using --with-default-api-version=v18
//  - HDF5 1.8.x compiled using --with-default-api-version=v16
//
// In this compatibility header file, we define a common HDF5 C API
// for all of the above versions, with the intent to minimize use
// of versioned #ifdefs in h5xx wrapper functions.
//

//
// Note for developers: If you make a change to h5xx wrapper, compile
// and run the test suite for *all* supported HDF5 library versions.
//

/**
 * if using HDF5 1.8.x, force HDF 1.8 API as needed
 */
#define H5Dcreate_vers   2
#define H5Dopen_vers     2
#define H5E_auto_t_vers  2
#define H5Eget_auto_vers 2
#define H5Eprint_vers    2
#define H5Eset_auto_vers 2
#define H5Ewalk_vers     2
#define H5Gcreate_vers   2
#define H5Gopen_vers     2

#include <hdf5.h>

/**
 * HDF5 C++ API (to be deprecated in HALMD)
 */
#include <H5Cpp.h>

#endif /* ! H5XX_COMPAT_HPP */
