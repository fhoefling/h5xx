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

#ifndef H5XX_DATASPACE_BOOST_ARRAY
#define H5XX_DATASPACE_BOOST_ARRAY

#include <h5xx/dataspace.hpp>

#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/mpl/and.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

namespace h5xx {

template <typename T>
inline typename boost::enable_if< boost::mpl::and_< is_array<T>, boost::is_fundamental<typename T::value_type> >, dataspace>::type
create_dataspace(T const& value)
{
    const int rank = 1;
    boost::array<hsize_t, rank> value_dims;
    value_dims[0] = value.size();
    return dataspace(value_dims);
}

} // namespace h5xx

#endif // ! H5XX_DATASPACE_BOOST_ARRAY
