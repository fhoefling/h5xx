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

/**
 * create dataset 'name' in given group or file with given size
 */
template <typename T>
typename boost::enable_if<boost::is_fundamental<T>, H5::DataSet>::type
create_dataset(
    H5::Group const& group
  , std::string const& name
  , hsize_t max_size=H5S_UNLIMITED)
{
        LOG_DEBUG("create dataset '" << name << "' in " << path(group));
        // scalar sample file dataspace
        hsize_t dim[1] = { 0 };
        hsize_t max_dim[1] = { max_size };
        H5::DataSpace ds(1, dim, max_dim);

        H5::DSetCreatPropList cparms;
        hsize_t chunk_dim[1] = { 1 };
        cparms.setChunk(1, chunk_dim);

        // remove dataset if it exists
        try {
            H5XX_NO_AUTO_PRINT(H5::GroupIException);
            group.unlink(name);
        }
        catch (H5::GroupIException const&) {}
        return group.createDataSet(name, H5xx::ctype<T>(), ds, cparms);
}

/**
 * write data to dataset at given index, default argument appends dataset
 */
template <typename T>
typename boost::enable_if<boost::is_fundamental<T> >::type
write(H5::DataSet& dataset, T const& data, hsize_t index=H5S_UNLIMITED)
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
            dataset.extend(dim);
        }
        else {
            start[0] = index;
        }
        dataspace.selectHyperslab(H5S_SELECT_SET, count, start, stride, block);

        dataset.write(&data, H5xx::ctype<T>(), H5S_SCALAR, dataspace);
}

/**
 * read data to dataset at given index
 */
template <typename T>
typename boost::enable_if<boost::is_fundamental<T>, hsize_t>::type
read(H5::DataSet& dataset, T* data, ssize_t index)
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
        dataset.read(data, H5xx::ctype<T>(), H5S_SCALAR, dataspace);
    }
    catch (H5::Exception const&) {
        throw std::runtime_error("HDF5 reader: failed to read scalar data");
    }

    return index;
}

#if 0

/**
 * create scalar sample dataset
 */
template <int dimension, typename float_type>
H5::DataSet hdf5<dimension, float_type>::create_scalar_dataset(
    H5::Group where
  , std::string const& name
  , float_type sample
  )
{
    H5::DataType const tid = H5xx::ctype<double>();

    // scalar sample file dataspace
    hsize_t dim[1] = { 0 };
    hsize_t max_dim[1] = { H5S_UNLIMITED };
    H5::DataSpace ds(1, dim, max_dim);

    H5::DSetCreatPropList cparms;
    hsize_t chunk_dim[1] = { 1 };
    cparms.setChunk(1, chunk_dim);

    return where.createDataSet(name, tid, ds, cparms);
}


/**
 * write vector sample dataset
 */
template <int dimension, typename float_type>
void hdf5<dimension, float_type>::write_vector_dataset(
    H5::DataSet dset
  , sample_vector_ptr sample
  )
{
    H5::DataType const tid = H5xx::ctype<float_type>();
    size_t const size = sample->size();

    // extend vector sample file dataspace
    H5::DataSpace ds(dset.getSpace());
    hsize_t dim[3];
    ds.getSimpleExtentDims(dim);
    hsize_t count[3]  = { 1, 1, 1 };
    hsize_t start[3]  = { dim[0], 0, 0 };
    hsize_t stride[3] = { 1, 1, 1 };
    hsize_t block[3]  = { 1, size, dimension };
    dim[0]++;
    ds.setExtentSimple(3, dim);
    ds.selectHyperslab(H5S_SELECT_SET, count, start, stride, block);

    // vector sample memory dataspace
    hsize_t dim_sample[2] = { size, dimension };
    H5::DataSpace ds_sample(2, dim_sample);

    dset.extend(dim);
    dset.write(sample->data(), tid, ds_sample, ds);
}

/**
 * read scalar sample dataset
 */
template <int dimension, typename float_type>
size_t hdf5<dimension, float_type>::read(H5::DataSet dset, float_type& sample)
{
    H5::DataSpace ds(dset.getSpace());

    if (!ds.isSimple()) {
        throw runtime_error("HDF5 scalar dataspace is not a simple dataspace");
    }
    if (ds.getSimpleExtentNdims() != 1) {
        throw runtime_error("HDF5 scalar dataspace has invalid dimensionality");
    }

    hsize_t dim[1];
    ds.getSimpleExtentDims(dim);

    ssize_t const len = dim[0];
    if ((offset_ >= len) || ((-offset_) > len)) {
        throw runtime_error("trajectory input sample number out of bounds");
    }
    size_t offset = (offset_ < 0) ? (offset_ + len) : offset_;

    hsize_t count[1]  = { 1 };
    hsize_t start[1]  = { offset };
    hsize_t stride[1] = { 1 };
    hsize_t block[1]  = { 1 };
    ds.selectHyperslab(H5S_SELECT_SET, count, start, stride, block);

    try {
        H5XX_NO_AUTO_PRINT(H5::Exception);
        dset.read(&sample, H5xx::ctype<float_type>(), H5S_SCALAR, ds);
    }
    catch (H5::Exception const&) {
        throw runtime_error("failed to read scalar sample from HDF5 trajectory input file");
    }

    return offset;
}


/**
 * create vector sample dataset
 */
template <int dimension, typename float_type>
H5::DataSet hdf5<dimension, float_type>::create_vector_dataset(
    H5::Group where
  , std::string const& name
  , sample_vector_ptr sample
  )
{
    H5::DataType const tid = H5xx::ctype<float_type>();
    size_t const size = sample->size();

    // vector sample file dataspace
    hsize_t dim[3] = { 0, size, dimension };
    hsize_t max_dim[3] = { H5S_UNLIMITED, size, dimension };
    H5::DataSpace ds(3, dim, max_dim);

    H5::DSetCreatPropList cparms;
    hsize_t chunk_dim[3] = { 1, size, dimension };
    cparms.setChunk(3, chunk_dim);
    // enable GZIP compression
    cparms.setDeflate(6);

    return where.createDataSet(name, tid, ds, cparms);
}

template <int dimension, typename float_type>
void hdf5<dimension, float_type>::write_vector_dataset(
    H5::DataSet dset
  , sample_vector_ptr sample
  )
{
    H5::DataType const tid = H5xx::ctype<float_type>();
    size_t const size = sample->size();

    // extend vector sample file dataspace
    H5::DataSpace ds(dset.getSpace());
    hsize_t dim[3];
    ds.getSimpleExtentDims(dim);
    hsize_t count[3]  = { 1, 1, 1 };
    hsize_t start[3]  = { dim[0], 0, 0 };
    hsize_t stride[3] = { 1, 1, 1 };
    hsize_t block[3]  = { 1, size, dimension };
    dim[0]++;
    ds.setExtentSimple(3, dim);
    ds.selectHyperslab(H5S_SELECT_SET, count, start, stride, block);

    // vector sample memory dataspace
    hsize_t dim_sample[2] = { size, dimension };
    H5::DataSpace ds_sample(2, dim_sample);

    dset.extend(dim);
    dset.write(sample->data(), tid, ds_sample, ds);
}

#endif // 0

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
