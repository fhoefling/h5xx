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
#include <h5xx/policy.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/mpl/and.hpp>
#include <boost/multi_array.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

namespace h5xx {

/**
 * Create dataset of multi-dimensional array type,
 * dataset object is instantiated temporarily.
 */
template <typename h5xxObject, typename T>
inline typename boost::enable_if<is_multi_array<T>, void>::type
create_dataset(h5xxObject const& object, std::string const& name, T const& value,
               h5xx::policy::dataset_creation_property_list dcpl = h5xx::policy::default_dataset_creation_property_list)
{
    dataset dset;
    create_dataset(dset, object, name, value);
}

/**
 * Create dataset of multi-dimensional array type,
 * dataset object is instantiated externally and
 * passed by reference ("dset").
 */
template <typename h5xxObject, typename T>
inline typename boost::enable_if<is_multi_array<T>, void>::type
create_dataset(dataset& dset, h5xxObject const& object, std::string const& name, T const& value,
               h5xx::policy::dataset_creation_property_list dcpl = h5xx::policy::default_dataset_creation_property_list)
{
    typedef typename T::element value_type;
    hid_t type_id = ctype<value_type>::hid(); // this ID must not be closed
    enum { rank = T::dimensionality };
    boost::array<hsize_t, rank> dims;
    std::copy(value.shape(), value.shape() + rank, dims.begin());
    // ---
    hid_t lcpl_id = H5P_DEFAULT;
    hid_t dcpl_id = H5P_DEFAULT;
    hid_t dapl_id = H5P_DEFAULT;
    dcpl_id = dcpl.get();
    // ---
    dset.create(object, name, type_id, dataspace(dims), lcpl_id, dcpl_id, dapl_id);
}

/**
 * Write multi_array (value) to dataset (dset).
 */
template <typename T>
inline typename boost::enable_if<is_multi_array<T>, void>::type
write_dataset(dataset& dset, T const& value)
{
    typedef typename T::element value_type;
    hid_t type_id = ctype<value_type>::hid();
    dset.write(type_id, value.origin());
}

/**
 * Write multi_array (value) to a dataset labeled "name" located at "object".
 */
template <typename h5xxObject, typename T>
inline typename boost::enable_if<is_multi_array<T>, void>::type
write_dataset(h5xxObject const& object, std::string const& name, T const& value)
{
    if (! exists_dataset(object, name))
    {
        throw error("dataset \"" + name + "\" of object \"" + get_name(object) + "\" does not exist");
//        create_dataset(object, name, value);
    }
    dataset dset(object, name);
    write_dataset(dset, value);
}

/**
 * Read a dataset labeled "name" to a Boost multi_array and return it.
 */
template <typename T, typename h5xxObject>
inline typename boost::enable_if<is_multi_array<T>, T>::type
read_dataset(h5xxObject const& object, std::string const& name)
{
    dataset dset(object, name);
    dataspace space(dset);
    enum { rank = T::dimensionality };
    if (!(space.rank() == rank)) {
        throw error("dataset \"" + name + "\" of object \"" + get_name(object) + "\" has mismatching dataspace");
    }

    boost::array<hsize_t, rank> dims = space.extents<rank>();
    boost::array<size_t, rank> shape;
    std::copy(dims.begin(), dims.begin() + rank, shape.begin());
    typedef typename T::element value_type;
    boost::multi_array<value_type, rank> value(shape);

    dset.read(ctype<value_type>::hid(), value.origin());

    return value;
}

/**
 * Read a dataset labeled "name" to a Boost multi_array and return it.
 */
template <typename T>
inline typename boost::enable_if<is_multi_array<T>, T>::type
read_dataset(dataset & dset, dataspace const& filespace, dataspace & memspace)
{
    enum { rank = T::dimensionality };
    if (!(memspace.rank() == rank)) {
        throw error("dataset \"" + dset.name() + "\" has mismatching dataspace");
    }

    boost::array<hsize_t, rank> dims = memspace.extents<rank>();
    boost::array<size_t, rank> shape;
    std::copy(dims.begin(), dims.begin() + rank, shape.begin());
    typedef typename T::element value_type;
    boost::multi_array<value_type, rank> value(shape);

    hid_t mem_space_id = memspace.hid(); //H5S_ALL;
    hid_t file_space_id = filespace.hid();
    hid_t xfer_plist_id = H5P_DEFAULT;
    dset.read(ctype<value_type>::hid(), value.origin(), mem_space_id, file_space_id, xfer_plist_id);

    return value;
}

} // namespace h5xx

#endif // ! H5XX_DATASET_MULTI_ARRAY
