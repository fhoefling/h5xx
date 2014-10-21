/*
 * Copyright © 2014 Felix Höfling
 * Copyright © 2014 Klaus Reuter
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

#ifndef H5XX_DATASPACE_MULTI_ARRAY
#define H5XX_DATASPACE_MULTI_ARRAY

#include <algorithm>

#include <h5xx/dataspace.hpp>

#include <boost/array.hpp>
#include <boost/multi_array.hpp>
#include <boost/utility/enable_if.hpp>

namespace h5xx {

template <typename T>
typename boost::enable_if<is_multi_array<T>, dataspace>::type
create_dataspace(T const& value)
{
    enum { rank = T::dimensionality };
    boost::array<hsize_t, rank> value_dims;
    std::copy(value.shape(), value.shape() + rank, value_dims.begin());
    return dataspace(value_dims);
}

} // namespace h5xx

#endif // ! H5XX_DATASPACE_MULTI_ARRAY
