/*
 * Copyright © 2014 Felix Höfling
 * Copyright © 2014  Manuel Dibak
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

#ifndef H5XX_ATTRIBUTE_MULTI_ARRAY
#define H5XX_ATTRIBUTE_MULTI_ARRAY

#include <algorithm>
#include <h5xx/ctype.hpp>
#include <h5xx/error.hpp>
#include <h5xx/exception.hpp>
#include <h5xx/utility.hpp>
#include <h5xx/attribute/utility.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/mpl/and.hpp>
#include <boost/multi_array.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

namespace h5xx {

template <typename h5xxObject, typename T>
inline typename boost::enable_if<is_multi_array<T>, void>::type
write_attribute(h5xxObject const& object, std::string const& name, T const& value)
{
    typedef typename T::element value_type;
    enum { rank = T::dimensionality };
    bool err = false;
    char const* attr_name = name.c_str();
    hid_t attr_id;
    if (exists_attribute(object, name))     //the attribute will be replaced if it already exists
    {
        delete_attribute(object, name);
    }
    hsize_t dim[rank];
    std::copy(value.shape(), value.shape() + rank, dim);
    hid_t space_id = H5Screate_simple(rank, dim, dim);
    hid_t type_id = ctype<value_type>::hid();       //this ID can not be closed
    err |= (attr_id = H5Acreate(object.hid(), attr_name, type_id, space_id, H5P_DEFAULT, H5P_DEFAULT)) < 0;
    err |= H5Sclose(space_id);
    if (err) {
        throw error("creating attribute");
    }

    err |= H5Awrite(attr_id, type_id, value.origin()) < 0;

    err |= H5Aclose(attr_id) < 0;
    if (err) {
        throw error("writing attribute \"" + name + "\" with ID " + boost::lexical_cast<std::string>(attr_id));
    }
}

/**
* read multi-dimensional array type attribute
*/

template <typename T, typename h5xxObject>
inline typename boost::enable_if<is_multi_array<T>, T>::type
read_attribute(h5xxObject const& object, std::string const& name)
{
    typedef typename T::element value_type;
    enum { rank = T::dimensionality };
    char const* attr_name = name.c_str();
    hid_t attr_id;
    bool err = false;
    hsize_t dim[rank];
    hsize_t maxdim[rank];

    if(!exists_attribute(object, attr_name)) {
        throw error("Attribute does not exist");
    }

    // open object
    err |= (attr_id = H5Aopen(object.hid(), attr_name, H5P_DEFAULT)) < 0;
    if (err) {
        throw error("opening atrribute \"" + name + "\" with ID " + boost::lexical_cast<std::string>(attr_id));
    }
    if (!has_simple_space(attr_id)) {
        throw error("Attribute \"" + name + "\" of object \"" + get_name(object) + "\" has incompatible dataspace");
    }

    hid_t attr_space = H5Aget_space(attr_id);
    if ( !has_rank<rank>(attr_space)) {
        throw error("Attribute has an invalid dataspace");
    }


    H5Sget_simple_extent_dims(attr_space, dim, maxdim);
    err |= H5Sclose(attr_space) < 0;

    boost::array<size_t, rank> shape;
    std::copy(dim, dim + rank, shape.begin());
    boost::multi_array<value_type, rank> value(shape);
    err |= (H5Aread(attr_id, ctype<value_type>::hid(), value.origin())) < 0;

    // close object
    err |= H5Aclose(attr_id) < 0;
    if (err) {
        throw error("reading atrribute \"" + name + "\" with ID " + boost::lexical_cast<std::string>(attr_id));
    }
    return value;
}
} //namespace h5xx

#endif // ! H5XX_ATTRIBUTE_MULTI_ARRAY
