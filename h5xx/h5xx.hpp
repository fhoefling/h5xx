/* HDF5 C++ extensions
 *
 * Copyright © 2008-2010  Peter Colberg and Felix Höfling
 * All rights reserved.
 *
 * This file is part of h5xx — a C++ wrapper for the HDF5 library.
 *
 * This software may be modified and distributed under the terms of the
 * 3-clause BSD license.  See accompanying file LICENSE for details.
 */

#ifndef H5XX_HPP
#define H5XX_HPP

namespace h5xx {
    enum { compression_level = 6 };
}

#include <h5xx/attribute.hpp>
#include <h5xx/ctype.hpp>
#include <h5xx/dataset.hpp>
#include <h5xx/chunked_dataset.hpp>
#include <h5xx/exception.hpp>
#include <h5xx/group.hpp>
#include <h5xx/utility.hpp>

#endif /* ! H5XX_HPP */
