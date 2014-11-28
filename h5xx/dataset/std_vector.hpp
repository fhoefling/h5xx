/*
 * Copyright © 2014 Felix Höfling
 * Copyright © 2014 Manuel Dibak
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

#ifndef H5XX_DATASET_STD_VECTOR
#define H5XX_DATASET_STD_VECTOR

#include <algorithm>
#include <vector>

#include <h5xx/ctype.hpp>
#include <h5xx/dataset/dataset.hpp>
#include <h5xx/dataspace.hpp>
#include <h5xx/error.hpp>
#include <h5xx/exception.hpp>
#include <h5xx/policy/storage.hpp>
#include <h5xx/utility.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/mpl/and.hpp>
//#include <boost/multi_array.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

namespace h5xx {



/**
 * create dataset from a std::vector of fundamental type
 **/
template <typename h5xxObject, typename T, typename StoragePolicy>
inline typename boost::enable_if< boost::mpl::and_< is_vector<T>, boost::is_fundamental<typename T::value_type> >, dataset>::type
create_dataset(h5xxObject const& object, std::string const& name, T const& value,
                   StoragePolicy const& storage_policy = StoragePolicy())
{
    typedef typename T::value_type value_type;
    hid_t type_id = ctype<value_type>::hid(); // this ID must not be closed
    boost::array<hsize_t, 1> dims = {{ value.size() }};
    return dataset(object, name, type_id, dataspace(dims), storage_policy);
}

/**
 * create dataset from a std::vector of fundamental type
 **/
template <typename h5xxObject, typename T>
inline typename boost::enable_if< boost::mpl::and_< is_vector<T>, boost::is_fundamental<typename T::value_type> >, dataset>::type
create_dataset(h5xxObject const& object, std::string const& name, T const& value)
{
    return create_dataset(object, name, value, h5xx::policy::storage::contiguous());
}



/**
 * Write std::vector data to an existing dataset specified by location and name.
 */
template <typename h5xxObject, typename T>
inline typename boost::enable_if< boost::mpl::and_< is_vector<T>, boost::is_fundamental<typename T::value_type> >, void>::type
write_dataset(h5xxObject const& object, std::string const& name, T const& value)
{
    dataset dset(object, name);
    write_dataset(dset, value);
}

/**
 * Write std::vector data to dataset.
 */
template <typename T>
inline typename boost::enable_if< boost::mpl::and_< is_vector<T>, boost::is_fundamental<typename T::value_type> >, void>::type
write_dataset(dataset& dset, T const& value)
{
    typedef typename T::value_type value_type;
    hid_t type_id = ctype<value_type>::hid();
    dset.write(type_id, &*value.begin());
}







///**
// * Write multiarray data to an existing dataset specified its location and name,
// * memory and file locations (hyperslabs) are passed via the dataspace objects.
// */
//template <typename h5xxObject, typename T>
//inline typename boost::enable_if<is_multi_array<T>, void>::type
//write_dataset(h5xxObject const& object, std::string const& name, T const& value,
//              dataspace const& memspace, dataspace const& filespace)
//{
//    dataset dset(object, name);
//    write_dataset(dset, value, memspace, filespace);
//}
//
///**
// * Write multiarray data to dataset, memory and file locations (hyperslabs)
// * are passed via the dataspace objects.
// */
//template <typename T>
//inline typename boost::enable_if<is_multi_array<T>, void>::type
//write_dataset(dataset& dset, T const& value, dataspace const& memspace, dataspace const& filespace)
//{
//    typedef typename T::element value_type;
//    hid_t type_id = ctype<value_type>::hid();
//    hid_t mem_space_id = memspace.hid(); //H5S_ALL;
//    hid_t file_space_id = filespace.hid();
//    hid_t xfer_plist_id = H5P_DEFAULT;
//    dset.write(type_id, value.origin(), mem_space_id, file_space_id, xfer_plist_id);
//}
//
///**
// * Write multiarray data to dataset specified its location and name, only the
// * file location (hyperslab) is given via a slice object.
// */
//template <typename h5xxObject, typename T>
//inline typename boost::enable_if<is_multi_array<T>, void>::type
//write_dataset(h5xxObject const& object, std::string const& name, T const& value, slice const& file_slice)
//{
//    dataset dset(object, name);
//    write_dataset(dset, value, file_slice);
//}
//
///**
// * Write multiarray data to dataset, only the file location (hyperslab) is given
// * via a slice object.
// */
//template <typename T>
//inline typename boost::enable_if<is_multi_array<T>, void>::type
//write_dataset(dataset& dset, T const& value, slice const& file_slice)
//{
//    // --- create memory dataspace for the complete input array
//    h5xx::dataspace memspace = h5xx::create_dataspace(value);
//    // --- create file dataspace and select the slice (hyperslab) from it
//    h5xx::dataspace filespace(dset);
//    filespace.select(file_slice);
//    write_dataset(dset, value, memspace, filespace);
//}
//
//
//
//
//template <typename h5xxObject, typename T>
//typename boost::enable_if<is_multi_array<T>, void>::type
//read_dataset(h5xxObject const& object, std::string const& name, T & array)
//{
//    dataset dset(object, name);
//    read_dataset(dset, array);
//}
//
//template <typename T>
//typename boost::enable_if<is_multi_array<T>, void>::type
//read_dataset(dataset & data_set, T & array)
//{
//    const int array_rank = T::dimensionality;
//    typedef typename T::element value_type;
//
//    // --- use temporary dataspace object to get the shape of the dataset
//    dataspace file_space(data_set);
//    if (!(file_space.rank() == array_rank))
//        H5XX_THROW("dataset \"" + get_name(data_set) + "\" and target array have mismatching dimensions");
//
//    boost::array<hsize_t, array_rank> file_dims = file_space.extents<array_rank>();
//
//    // --- clear array - TODO check if this feature is necessary/wanted
//    boost::array<size_t, array_rank> array_zero;
//    array_zero.assign(0);
//    array.resize(array_zero);
//
//    // --- resize array to match the dataset - TODO check if this feature is necessary/wanted
//    boost::array<size_t, array_rank> array_shape;
//    std::copy(file_dims.begin(), file_dims.begin() + array_rank, array_shape.begin());
//    array.resize(array_shape);
//
//    hid_t mem_space_id = H5S_ALL;
//    hid_t file_space_id = H5S_ALL;
//    hid_t xfer_plist_id = H5P_DEFAULT;
//
//    data_set.read(ctype<value_type>::hid(), array.origin(), mem_space_id, file_space_id, xfer_plist_id);
//}
//
//
//template <typename h5xxObject, typename T>
//typename boost::enable_if<is_multi_array<T>, void>::type
//read_dataset(h5xxObject const& object, std::string const& name, T & array, slice const& file_slice)
//{
//    dataset data_set(object, name);
//    read_dataset(data_set, array, file_slice);
//}
//
//template <typename T>
//typename boost::enable_if<is_multi_array<T>, void>::type
//read_dataset(dataset & data_set, T & array, slice const& file_slice)
//{
//    // --- create memory dataspace for the complete input array
//    h5xx::dataspace memspace = h5xx::create_dataspace(array);
//    // --- create file dataspace and select the slice (hyperslab) from it
//    h5xx::dataspace filespace(data_set);
//    filespace.select(file_slice);
//    // ---
//    read_dataset(data_set, array, memspace, filespace);
//}
//
//template <typename T>
//typename boost::enable_if<is_multi_array<T>, void>::type
//read_dataset(dataset & data_set, T & array, dataspace const& memspace, dataspace const& filespace)
//{
//    // --- disabled this check, it is orthogonal to a useful feature (eg read from 2D dataset into 1D array)
////    const int array_rank = T::dimensionality;
////    if (!(memspace.rank() == array_rank)) {
////        throw error("memory dataspace and array rank do not match");
////    }
//
//    if (static_cast<hsize_t>(filespace.get_select_npoints()) > array.num_elements())
//        H5XX_THROW("target array does not provide enough space to store slice");
//
//    hid_t mem_space_id = memspace.hid(); //H5S_ALL;
//    hid_t file_space_id = filespace.hid();
//    hid_t xfer_plist_id = H5P_DEFAULT;
//
//    typedef typename T::element value_type;
//    data_set.read(ctype<value_type>::hid(), array.origin(), mem_space_id, file_space_id, xfer_plist_id);
//}


} // namespace h5xx

#endif // ! H5XX_DATASET_STD_VECTOR
