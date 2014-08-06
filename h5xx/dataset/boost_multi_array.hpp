/*
 * Copyright © 2014 Felix Höfling
 * Copyright © 2014 Manuel Dibak
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

#ifndef H5XX_DATASET_MULTI_ARRAY
#define H5XX_DATASET_MULTI_ARRAY

#include <algorithm>

#include <h5xx/dataset/dataset.hpp>
#include <h5xx/ctype.hpp>
#include <h5xx/dataspace.hpp>
#include <h5xx/error.hpp>
#include <h5xx/exception.hpp>
#include <h5xx/utility.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/mpl/and.hpp>
#include <boost/multi_array.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

namespace h5xx {

/**
 * create dataset of multi-dimensional array type
 */
template <typename h5xxObject, typename T>
inline typename boost::enable_if<is_multi_array<T>, void>::type
create_dataset(dataset& dset, h5xxObject const& object, std::string const& name, T const& value)
{
    typedef typename T::element value_type;
    hid_t type_id = ctype<value_type>::hid(); // this ID must not be closed

    enum { rank = T::dimensionality };
    boost::array<hsize_t, rank> dims;
    std::copy(value.shape(), value.shape() + rank, dims.begin());

    dset.create(object, name, type_id, dataspace(dims));
}

/**
 * create dataset of multi-dimensional array type
 */
template <typename h5xxObject, typename T>
inline typename boost::enable_if<is_multi_array<T>, void>::type
create_dataset(h5xxObject const& object, std::string const& name, T const& value)
{
    typedef typename T::element value_type;
    hid_t type_id = ctype<value_type>::hid(); // this ID must not be closed
    enum { rank = T::dimensionality };
    boost::array<hsize_t, rank> dims;
    std::copy(value.shape(), value.shape() + rank, dims.begin());
    dataset dset;
    dset.create(object, name, type_id, dataspace(dims));
}

/**
 * write dataset of multi-dimensional array type
 */
template <typename T>
inline typename boost::enable_if<is_multi_array<T>, void>::type
write_dataset(dataset& dset, T const& value)
{
    typedef typename T::element value_type;
    hid_t type_id = ctype<value_type>::hid();       // this ID must not be closed

    dset.write(type_id, value.origin());
}

/**
 * write dataset of multi-dimensional array type
 */
template <typename h5xxObject, typename T>
inline typename boost::enable_if<is_multi_array<T>, void>::type
write_dataset(h5xxObject const& object, std::string const& name, T const& value)
{
    typedef typename T::element value_type;
    hid_t type_id = ctype<value_type>::hid();       // this ID must not be closed
    if (! exists_dataset(object, name))
        create_dataset(object, name, value);
    dataset dset(object, name);
    dset.write(type_id, value.origin());
}


/**
 * read dataset of multi-dimensional array type
 */
template <typename T, typename h5xxObject>
inline typename boost::enable_if<is_multi_array<T>, T>::type
read_dataset(h5xxObject const& object, std::string const& name)
{
    typedef typename T::element value_type;
    enum { rank = T::dimensionality };

    // open object
    dataset dset(object, name);

    // check if rank of dataspace and rank of array to be returned are matching
    dataspace space(dset);

    if (!(space.rank() == rank)) {
        throw error("attribute \"" + name + "\" of object \"" + get_name(object) + "\" has mismatching dataspace");
    }

    // get extents of dataspace
    boost::array<hsize_t, rank> dims = space.extents<rank>();

    // create boost::multi_array of given extents for use as buffer
    boost::array<size_t, rank> shape;
    std::copy(dims.begin(), dims.begin() + rank, shape.begin());
    boost::multi_array<value_type, rank> value(shape);

    // read attribute to buffer
    dset.read(ctype<value_type>::hid(), value.origin());

    return value;
}


} // namespace h5xx

#endif // ! H5XX_DATASET_MULTI_ARRAY
