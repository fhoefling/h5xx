/* HDF5 C++ extensions
 *
 * Copyright © 2008-2009  Peter Colberg
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

#ifndef HALMD_UTIL_H5XX_DATASET_HPP
#define HALMD_UTIL_H5XX_DATASET_HPP

#define H5E_auto_t_vers 2
#include <H5Cpp.h>

#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/mpl/and.hpp>
#include <boost/multi_array.hpp>
#include <boost/ref.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <vector>

#include <H5xx/attribute.hpp>
#include <H5xx/util.hpp>
#include <halmd/io/logger.hpp>

namespace H5xx
{

enum { compression_level = 6 };

/**
 * create dataset 'name' in given group with given size
 */
// first argument could be H5::CommonFG if the LOG_DEBUG line would be omitted
// chunks of scalars
template <typename T>
typename boost::enable_if<boost::is_fundamental<T>, H5::DataSet>::type
create_dataset(
    H5::Group const& group
  , std::string const& name
  , hsize_t max_size=H5S_UNLIMITED)
{
    LOG_DEBUG("create dataset '" << name << "' in " << path(group));
    // scalar sample file dataspace
    hsize_t dim[1] = { max_size == H5S_UNLIMITED ? 0 : max_size };
    hsize_t max_dim[1] = { max_size };
    hsize_t chunk_dim[1] = { 1 };

    H5::DataSpace dataspace(1, dim, max_dim);
    H5::DSetCreatPropList cparms;
    cparms.setChunk(1, chunk_dim);
    cparms.setDeflate(compression_level);    // enable GZIP compression

    // remove dataset if it exists
    try {
        H5XX_NO_AUTO_PRINT(H5::GroupIException);
        group.unlink(name);
    }
    catch (H5::GroupIException const&) {}
    return group.createDataSet(name, ctype<T>(), dataspace, cparms);
}

// chunks of fixed-size arrays
template <typename T>
typename boost::enable_if<boost::mpl::and_<
        is_boost_array<T>, boost::is_fundamental<typename T::value_type>
    >, H5::DataSet>::type
create_dataset(
    H5::Group const& group
  , std::string const& name
  , hsize_t max_size=H5S_UNLIMITED)
{
    LOG_DEBUG("create dataset '" << name << "' in " << path(group));

    // file dataspace holding max_size chunks of fixed-size arrays
    hsize_t dim[2] = { max_size == H5S_UNLIMITED ? 0 : max_size, T::static_size };
    hsize_t max_dim[2] = { max_size, T::static_size };
    hsize_t chunk_dim[2] = { 1, T::static_size };

    H5::DataSpace dataspace(2, dim, max_dim);
    H5::DSetCreatPropList cparms;
    cparms.setChunk(2, chunk_dim);
    cparms.setDeflate(compression_level);    // enable GZIP compression

    // remove dataset if it exists
    try {
        H5XX_NO_AUTO_PRINT(H5::GroupIException);
        group.unlink(name);
    }
    catch (H5::GroupIException const&) {}
    return group.createDataSet(name, ctype<typename T::value_type>(), dataspace, cparms);
}

// chunks of multi-arrays of fixed rank
template <typename T>
typename boost::enable_if<is_boost_multi_array<T>, H5::DataSet>::type
create_dataset(
    H5::Group const& group
  , std::string const& name
  , typename T::size_type const* shape
  , hsize_t max_size=H5S_UNLIMITED)
{
    LOG_DEBUG("create dataset '" << name << "' in " << path(group));

    // file dataspace holding max_size multi_array chunks of fixed rank
    enum { rank = T::dimensionality };
    boost::array<hsize_t, rank+1> dim, max_dim, chunk_dim;
    std::copy(shape, shape + rank, dim.begin() + 1);
    max_dim = dim;
    chunk_dim = dim;
    dim[0] = (max_size == H5S_UNLIMITED) ? 0 : max_size;
    max_dim[0] = max_size;
    chunk_dim[0] = 1;

    H5::DataSpace dataspace(dim.size(), dim.data(), max_dim.data());
    H5::DSetCreatPropList cparms;
    cparms.setChunk(chunk_dim.size(), chunk_dim.data());
    cparms.setDeflate(compression_level);    // enable GZIP compression

    // remove dataset if it exists
    try {
        H5XX_NO_AUTO_PRINT(H5::GroupIException);
        group.unlink(name);
    }
    catch (H5::GroupIException const&) {}
    return group.createDataSet(name, ctype<typename T::element>(), dataspace, cparms);
}

/**
 * write data to dataset at given index, default argument appends dataset
 */
// chunks of scalars
template <typename T>
typename boost::enable_if<boost::is_fundamental<T> >::type
write(H5::DataSet const& dataset, T const& data, hsize_t index=H5S_UNLIMITED)
{
    if (index == H5S_UNLIMITED) {
        LOG_DEBUG("append to dataset " << path(dataset));
    }
    else {
        LOG_DEBUG("write to dataset " << path(dataset) << " at " << index);
    }
    H5::DataSpace dataspace(dataset.getSpace());

    // select hyperslab
    hsize_t dim[1];
    dataspace.getSimpleExtentDims(dim);
    hsize_t count[1]  = { 1 };
    hsize_t start[1]  = { dim[0] };
    hsize_t stride[1] = { 1 };
    hsize_t block[1]  = { 1 };
    if (index == H5S_UNLIMITED) {
        // extend dataspace to append another scalar sample
        dim[0]++;
        dataspace.setExtentSimple(1, dim);
        try {
            H5XX_NO_AUTO_PRINT(H5::DataSpaceIException);
            dataset.extend(dim);
        }
        catch (H5::DataSpaceIException const&) {
            throw std::runtime_error("HDF5 writer: fixed-size dataset cannot be extended");
        }
    }
    else {
        start[0] = index;
    }
    dataspace.selectHyperslab(H5S_SELECT_SET, count, start, stride, block);

    dataset.write(&data, ctype<T>(), H5S_SCALAR, dataspace);
}

// chunks of fixed-size arrays
template <typename T>
typename boost::enable_if<boost::mpl::and_<
    is_boost_array<T>, boost::is_fundamental<typename T::value_type>
> >::type
write(H5::DataSet const& dataset, T const& data, hsize_t index=H5S_UNLIMITED)
{
    if (index == H5S_UNLIMITED) {
        LOG_DEBUG("append to dataset " << path(dataset));
    }
    else {
        LOG_DEBUG("write to dataset " << path(dataset) << " at " << index);
    }
    H5::DataSpace dataspace(dataset.getSpace());

    // select hyperslab
    hsize_t dim[2];
    dataspace.getSimpleExtentDims(dim);
    hsize_t count[2]  = { 1, 1 };
    hsize_t start[2]  = { dim[0], 0 };
    hsize_t stride[2] = { 1, 1 };
    hsize_t block[2]  = { 1, T::static_size };
    if (index == H5S_UNLIMITED) {
        // extend dataspace to append another chunk
        dim[0]++;
        dataspace.setExtentSimple(2, dim);
        try {
            H5XX_NO_AUTO_PRINT(H5::DataSetIException);
            dataset.extend(dim);
        }
        catch (H5::DataSetIException const&) {
            throw std::runtime_error("HDF5 writer: fixed-size dataset cannot be extended");
        }
    }
    else {
        start[0] = index;
    }
    dataspace.selectHyperslab(H5S_SELECT_SET, count, start, stride, block);

    // memory dataspace
    hsize_t dim_mem[1] = { T::static_size };
    H5::DataSpace mem_dataspace(1, dim_mem);

    dataset.write(data.data(), ctype<typename T::value_type>(), mem_dataspace, dataspace);
}

// chunks of multi_arrays of fixed rank
template <typename T>
typename boost::enable_if<is_boost_multi_array<T> >::type
write(H5::DataSet const& dataset, T const& data, hsize_t index=H5S_UNLIMITED)
{
    if (index == H5S_UNLIMITED) {
        LOG_DEBUG("append to dataset " << path(dataset));
    }
    else {
        LOG_DEBUG("write to dataset " << path(dataset) << " at " << index);
    }
    H5::DataSpace dataspace(dataset.getSpace());

    // select hyperslab of multi_array chunk
    enum { rank = T::dimensionality };
    boost::array<hsize_t, rank+1> dim, count, start, stride, block;
    dataspace.getSimpleExtentDims(dim.data());
    std::fill(count.begin(), count.end(), 1);
    start[0] = dim[0];
    std::fill(start.begin() + 1, start.end(), 0);
    std::fill(stride.begin(), stride.end(), 1);
    block[0] = 1;
    std::copy(data.shape(), data.shape() + rank, block.begin() + 1);

    if (index == H5S_UNLIMITED) {
        // extend dataspace to append another chunk
        dim[0]++;
        dataspace.setExtentSimple(dim.size(), dim.data());
        try {
            H5XX_NO_AUTO_PRINT(H5::DataSetIException);
            dataset.extend(dim.data());
        }
        catch (H5::DataSetIException const&) {
            throw std::runtime_error("HDF5 writer: fixed-size dataset cannot be extended");
        }
    }
    else {
        start[0] = index;
    }
    dataspace.selectHyperslab(H5S_SELECT_SET, count.data(), start.data(), stride.data(), block.data());

    // memory dataspace
    H5::DataSpace mem_dataspace(rank, block.begin() + 1);

    dataset.write(data.data(), ctype<typename T::element>(), mem_dataspace, dataspace);
}

/**
 * read data to dataset at given index
 */
template <typename T>
typename boost::enable_if<boost::is_fundamental<T>, hsize_t>::type
read(H5::DataSet const& dataset, T* data, ssize_t index)
{
    LOG_DEBUG("read from dataset " << path(dataset) << " at " << index);

    H5::DataSpace dataspace(dataset.getSpace());
    if (!has_type<T>(dataset) || !has_rank<1>(dataspace)) {
        throw std::runtime_error("HDF5 reader: dataset has incompatible dataspace");
    }

    hsize_t dim[1];
    dataspace.getSimpleExtentDims(dim);

    ssize_t const len = dim[0];
    if ((index >= len) || ((-index) > len)) {
        throw std::runtime_error("HDF5 reader: index out of bounds");
    }
    index = (index < 0) ? (index + len) : index;

    hsize_t count[1]  = { 1 };
    hsize_t start[1]  = { index };
    hsize_t stride[1] = { 1 };
    hsize_t block[1]  = { 1 };
    dataspace.selectHyperslab(H5S_SELECT_SET, count, start, stride, block);

    try {
        H5XX_NO_AUTO_PRINT(H5::Exception);
        dataset.read(data, ctype<T>(), H5S_SCALAR, dataspace);
    }
    catch (H5::Exception const&) {
        throw std::runtime_error("HDF5 reader: failed to read scalar data");
    }

    return index;
}

/**
 * return first dimension of dataset
 */
inline hsize_t elements(H5::DataSet ds)
{
    return ds.getSpace().getSimpleExtentNpoints();
}

/**
 * helper functions for convenience
 *
 * we pass the data via pointer to make it transparent for the client code
 * that we store a const reference
 */
template <typename T>
boost::function<void ()> make_dataset_writer(H5::DataSet const& dataset, T const* data)
{
    return boost::bind(&write<T>, dataset, boost::cref(*data), H5S_UNLIMITED);
}

template <typename T>
boost::function<void (hsize_t)> make_dataset_write_at(H5::DataSet const& dataset, T const* data)
{
    return boost::bind(&write<T>, dataset, boost::cref(*data), _1);
}

template <typename T>
boost::function<void ()> make_dataset_writer(
    H5::Group const& group, std::string const& name, T const* data)
{
    H5::DataSet dataset = create_dataset<T>(group, name);
    return make_dataset_writer(dataset, data);
}

template <typename T>
boost::function<void (hsize_t)> make_dataset_write_at(
    H5::Group const& group, std::string const& name, T const* data)
{
    H5::DataSet dataset = create_dataset<T>(group, name);
    return make_dataset_write_at(dataset, data);
}
} // namespace H5xx

#endif /* ! HALMD_UTIL_H5XX_DATASET_HPP */
