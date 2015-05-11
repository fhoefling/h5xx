/* HDF5 C++ extensions
 *
 * Copyright © 2008-2015 Felix Höfling
 * Copyright © 2014-2015 Klaus Reuter
 * Copyright © 2014      Manuel Dibak
 * Copyright © 2008-2010 Peter Colberg
 *
 * This file is part of h5xx.
 *
 * h5xx is free software: you can redistribute it and/or modify
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

#include <hdf5.h>

#ifndef H5XX_HPP
#define H5XX_HPP

namespace h5xx {
    enum { compression_level = 6 };
}

#include <h5xx/attribute.hpp>
#include <h5xx/ctype.hpp>
#include <h5xx/dataset.hpp>
#include <h5xx/datatype.hpp>
#include <h5xx/dataspace.hpp>
#include <h5xx/slice.hpp>
//#include <h5xx/chunked_dataset.hpp>
#include <h5xx/exception.hpp>
#include <h5xx/file.hpp>
#include <h5xx/group.hpp>
#include <h5xx/utility.hpp>

#endif /* ! H5XX_HPP */
