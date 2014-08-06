/*
 * Copyright © 2014      Klaus Reuter
 * Copyright © 2013-2014 Felix Höfling
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

#include <h5xx/ctype.hpp>

#include <boost/array.hpp>
#include <boost/multi_array.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

#ifndef H5XX_DATATYPE_DATATYPE_HPP
#define H5XX_DATATYPE_DATATYPE_HPP

namespace h5xx {

/**
 * Wrapper class for the HDF5 datatype.
 */
class datatype
{
public:
    /** default constructor */
    datatype() : type_id_(-1) {}

    /** create datatype object from a HDF5 type_id */
    datatype(hid_t type_id) : type_id_(type_id) {}

    /** create datatype object from a Boost multi_array */
    template <class T>
    datatype(T array, typename boost::enable_if<is_multi_array<T> >::type* dummy = 0);

    /** return the HDF5 type ID */
    hid_t get_type_id() const;

protected:
    /** HDF5 type ID */
    hid_t type_id_;
};

template <class T>
datatype::datatype(T array, typename boost::enable_if<is_multi_array<T> >::type* dummy)
{
    typedef typename T::element value_type;
    type_id_ = ctype<value_type>::hid();
}

hid_t datatype::get_type_id() const
{
    return type_id_;
}

} // namespace h5xx

#endif // ! H5XX_DATATYPE_DATATYPE_HPP
