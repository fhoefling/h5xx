/*
 * Copyright © 2008-2011  Peter Colberg and Felix Höfling
 * All rights reserved.
 *
 * This file is part of h5xx — a C++ wrapper for the HDF5 library.
 *
 * This software may be modified and distributed under the terms of the
 * 3-clause BSD license.  See accompanying file LICENSE for details.
 */

#ifndef H5XX_CHUNKED_DATASET_HPP
#define H5XX_CHUNKED_DATASET_HPP

#include <h5xx/attribute.hpp>
#include <h5xx/property.hpp>
#include <h5xx/utility.hpp>

#include <boost/array.hpp>
#include <boost/mpl/and.hpp>
#include <boost/multi_array.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

#include <algorithm>
#include <vector>

namespace h5xx {

namespace detail {

// http://www.hdfgroup.org/training/HDFtraining/UsersGuide/Perform.fm2.html
// It is recommended that the chunk size be at least 8K bytes.
enum { CHUNK_MIN_SIZE = 8092 };

/**
 * create chunked dataset 'name' in given group/file with given size
 *
 * This function creates missing intermediate groups.
 */
// generic case: some fundamental type and a shape of arbitrary rank
template <typename T, int rank>
inline typename boost::enable_if<boost::is_fundamental<T>, H5::DataSet>::type
create_chunked_dataset(
    H5::CommonFG const& fg
  , std::string const& name
  , hsize_t const* shape
  , hsize_t max_size=H5S_UNLIMITED)
{
    H5::IdComponent const& loc = dynamic_cast<H5::IdComponent const&>(fg);

    // file dataspace holding max_size multi_array chunks of fixed rank
    boost::array<hsize_t, rank+1> dim, max_dim, chunk_dim;
    std::copy(shape, shape + rank, dim.begin() + 1);
    std::copy(shape, shape + rank, max_dim.begin() + 1);
    std::copy(shape, shape + rank, chunk_dim.begin() + 1);
    dim[0] = (max_size == H5S_UNLIMITED) ? 0 : max_size;
    max_dim[0] = max_size;
    chunk_dim[0] = 1;

    // increase outermost dimension of chunk by powers of two until size of
    // dataset is equal to or greater than recommended minimum chunk size
    while (std::accumulate(chunk_dim.begin(), chunk_dim.end(), sizeof(T), std::multiplies<hsize_t>()) < CHUNK_MIN_SIZE) {
        chunk_dim[0] *= 2;
    }
    chunk_dim[0] = std::min(chunk_dim[0], max_dim[0]);

    H5::DataSpace dataspace(dim.size(), &*dim.begin(), &*max_dim.begin());
    H5::DSetCreatPropList cparms;
    cparms.setChunk(chunk_dim.size(), &*chunk_dim.begin());
    cparms.setDeflate(compression_level);    // enable GZIP compression

    // remove dataset if it exists
    H5E_BEGIN_TRY {
        H5Ldelete(loc.getId(), name.c_str(), H5P_DEFAULT);
    } H5E_END_TRY

    H5::PropList pl = create_intermediate_group_property();
    hid_t dataset_id = H5Dcreate(
        loc.getId(), name.c_str(), ctype<T>::hid(), dataspace.getId()
      , pl.getId(), cparms.getId(), H5P_DEFAULT
    );
    if (dataset_id < 0) {
        throw error("failed to create dataset \"" + name + "\"");
    }
    return H5::DataSet(dataset_id);
}

/**
 * write data to chunked dataset at given index, default argument appends to dataset
 */
// generic case: some fundamental type and a pointer to the contiguous array of data
// size and shape are taken from the dataset
template <typename T, int rank>
inline typename boost::enable_if<boost::is_fundamental<T>, void>::type
write_chunked_dataset(H5::DataSet const& dataset, T const* data, hsize_t index=H5S_UNLIMITED)
{
    H5::DataSpace dataspace(dataset.getSpace());
    if (!has_rank<rank+1>(dataspace)) {
        throw std::runtime_error("HDF5 writer: dataset has incompatible dataspace");
    }

    // select hyperslab of multi_array chunk
    boost::array<hsize_t, rank+1> dim, count, start, stride, block;
    dataspace.getSimpleExtentDims(&*dim.begin());
    std::fill(count.begin(), count.end(), 1);
    start[0] = dim[0];
    std::fill(start.begin() + 1, start.end(), 0);
    std::fill(stride.begin(), stride.end(), 1);
    block = dim;
    block[0] = 1;

    if (index == H5S_UNLIMITED) {
        // extend dataspace to append another chunk
        dim[0]++;
        dataspace.setExtentSimple(dim.size(), &*dim.begin());
        try {
            H5XX_NO_AUTO_PRINT(H5::DataSetIException);
            dataset.extend(&*dim.begin());
        }
        catch (H5::DataSetIException const&) {
            throw std::runtime_error("HDF5 writer: fixed-size dataset cannot be extended");
        }
    }
    else {
        start[0] = index;
    }
    dataspace.selectHyperslab(H5S_SELECT_SET, &*count.begin(), &*start.begin(), &*stride.begin(), &*block.begin());

    // memory dataspace
    H5::DataSpace mem_dataspace(rank, block.begin() + 1);

    dataset.write(data, ctype<T>::hid(), mem_dataspace, dataspace);
}

/**
 * read data from chunked dataset at given index
 */
// generic case: some (fundamental) type and a pointer to the contiguous array of data
// size and shape are taken from the dataset
template <typename T, int rank>
inline typename boost::enable_if<boost::is_fundamental<T>, hsize_t>::type
read_chunked_dataset(H5::DataSet const& dataset, T* data, ssize_t index)
{
    H5::DataSpace dataspace(dataset.getSpace());
    if (!has_rank<rank+1>(dataspace)) {
        throw std::runtime_error("HDF5 reader: dataset has incompatible dataspace");
    }

    boost::array<hsize_t, rank+1> dim;
    dataspace.getSimpleExtentDims(&*dim.begin());

    ssize_t const len = dim[0];
    if ((index >= len) || ((-index) > len)) {
        throw std::runtime_error("HDF5 reader: index out of bounds");
    }
    index = (index < 0) ? (index + len) : index;

    boost::array<hsize_t, rank+1> count, start, stride, block;
    std::fill(count.begin(), count.end(), 1);
    start[0] = index;
    std::fill(start.begin() + 1, start.end(), 0);
    std::fill(stride.begin(), stride.end(), 1);
    block = dim;
    block[0] = 1;

    dataspace.selectHyperslab(H5S_SELECT_SET, &*count.begin(), &*start.begin(), &*stride.begin(), &*block.begin());

    // memory dataspace
    H5::DataSpace mem_dataspace(rank, dim.begin() + 1);

    try {
        H5XX_NO_AUTO_PRINT(H5::Exception);
        dataset.read(data, ctype<T>::hid(), mem_dataspace, dataspace);
    }
    catch (H5::Exception const&) {
        throw std::runtime_error("HDF5 reader: failed to read multidimensional array data");
    }

    return index;
}

} // namespace detail

//
// chunks of scalars
//
template <typename T>
inline typename boost::enable_if<boost::is_fundamental<T>, H5::DataSet>::type
create_chunked_dataset(
    H5::CommonFG const& fg
  , std::string const& name
  , hsize_t max_size=H5S_UNLIMITED)
{
    return detail::create_chunked_dataset<T, 0>(fg, name, NULL, max_size);
}

template <typename T>
inline typename boost::enable_if<boost::is_fundamental<T>, void>::type
write_chunked_dataset(H5::DataSet const& dataset, T const& data, hsize_t index=H5S_UNLIMITED)
{
    detail::write_chunked_dataset<T, 0>(dataset, &data, index);
}

template <typename T>
inline typename boost::enable_if<boost::is_fundamental<T>, hsize_t>::type
read_chunked_dataset(H5::DataSet const& dataset, T& data, ssize_t index)
{
    return detail::read_chunked_dataset<T, 0>(dataset, &data, index);
}

//
// chunks of fixed-size arrays
//
template <typename T>
inline typename boost::enable_if<boost::mpl::and_<
        is_array<T>, boost::is_fundamental<typename T::value_type>
    >, H5::DataSet>::type
create_chunked_dataset(
    H5::CommonFG const& fg
  , std::string const& name
  , hsize_t max_size=H5S_UNLIMITED)
{
    typedef typename T::value_type value_type;
    enum { rank = 1 };
    hsize_t shape[1] = { T::static_size };
    return detail::create_chunked_dataset<value_type, rank>(fg, name, shape, max_size);
}

template <typename T>
inline typename boost::enable_if<boost::mpl::and_<
        is_array<T>, boost::is_fundamental<typename T::value_type>
    >, void>::type
write_chunked_dataset(H5::DataSet const& dataset, T const& data, hsize_t index=H5S_UNLIMITED)
{
    typedef typename T::value_type value_type;
    enum { rank = 1 };
    if (!has_extent<T, 1>(dataset))
    {
        throw std::runtime_error("HDF5 writer: dataset has incompatible dataspace");
    }
    detail::write_chunked_dataset<value_type, rank>(dataset, &*data.begin(), index);
}

template <typename T>
inline typename boost::enable_if<boost::mpl::and_<
        is_array<T>, boost::is_fundamental<typename T::value_type>
    >, hsize_t>::type
read_chunked_dataset(H5::DataSet const& dataset, T& data, ssize_t index)
{
    typedef typename T::value_type value_type;
    enum { rank = 1 };
    return detail::read_chunked_dataset<value_type, rank>(dataset, &*data.begin(), index);
}

//
// chunks of multi-arrays of fixed rank
//
template <typename T>
inline typename boost::enable_if<is_multi_array<T>, H5::DataSet>::type
create_chunked_dataset(
    H5::CommonFG const& fg
  , std::string const& name
  , typename T::size_type const* shape
  , hsize_t max_size=H5S_UNLIMITED)
{
    typedef typename T::element value_type;
    enum { rank = T::dimensionality };
    // convert T::size_type to hsize_t
    boost::array<hsize_t, rank> shape_;
    std::copy(shape, shape + rank, shape_.begin());
    return detail::create_chunked_dataset<value_type, rank>(fg, name, &*shape_.begin(), max_size);
}

template <typename T>
inline typename boost::enable_if<is_multi_array<T>, void>::type
write_chunked_dataset(H5::DataSet const& dataset, T const& data, hsize_t index=H5S_UNLIMITED)
{
    typedef typename T::element value_type;
    enum { rank = T::dimensionality };
    if (!has_extent<T, 1>(dataset, data.shape()))
    {
        throw std::runtime_error("HDF5 writer: dataset has incompatible dataspace");
    }
    detail::write_chunked_dataset<value_type, rank>(dataset, data.origin(), index);
}

/** read chunk of multi_array data, resize/reshape result array if necessary */
template <typename T>
inline typename boost::enable_if<is_multi_array<T>, hsize_t>::type
read_chunked_dataset(H5::DataSet const& dataset, T& data, ssize_t index)
{
    typedef typename T::element value_type;
    enum { rank = T::dimensionality };

    // determine extent of data space
    H5::DataSpace dataspace(dataset.getSpace());
    if (!has_rank<rank+1>(dataspace)) {
        throw std::runtime_error("HDF5 reader: dataset has incompatible dataspace");
    }
    boost::array<hsize_t, rank+1> dim;
    dataspace.getSimpleExtentDims(&*dim.begin());

    // resize result array if necessary, may allocate new memory
    if (!std::equal(dim.begin() + 1, dim.end(), data.shape())) {
        boost::array<size_t, rank> shape;
        std::copy(dim.begin() + 1, dim.end(), shape.begin());
        data.resize(shape);
    }

    return detail::read_chunked_dataset<value_type, rank>(dataset, data.origin(), index);
}

//
// chunks of vector containers holding scalars
//
// pass length of vector as third parameter
template <typename T>
inline typename boost::enable_if<boost::mpl::and_<
        is_vector<T>, boost::is_fundamental<typename T::value_type>
    >, H5::DataSet>::type
create_chunked_dataset(
    H5::CommonFG const& fg
  , std::string const& name
  , typename T::size_type size
  , hsize_t max_size=H5S_UNLIMITED)
{
    typedef typename T::value_type value_type;
    hsize_t shape[1] = { size };
    return detail::create_chunked_dataset<value_type, 1>(fg, name, shape, max_size);
}

template <typename T>
inline typename boost::enable_if<boost::mpl::and_<
        is_vector<T>, boost::is_fundamental<typename T::value_type>
    >, void>::type
write_chunked_dataset(H5::DataSet const& dataset, T const& data, hsize_t index=H5S_UNLIMITED)
{
    typedef typename T::value_type value_type;

    // assert data.size() corresponds to dataspace extents
    if (has_rank<2>(dataset)) {
        hsize_t dim[2];
        dataset.getSpace().getSimpleExtentDims(dim);
        if (data.size() != dim[1]) {
            throw std::runtime_error("HDF5 writer: dataset has incompatible dataspace");
        }
    }

    detail::write_chunked_dataset<value_type, 1>(dataset, &*data.begin(), index);
}

/** read chunk of vector container with scalar data, resize/reshape result array if necessary */
template <typename T>
inline typename boost::enable_if<boost::mpl::and_<
        is_vector<T>, boost::is_fundamental<typename T::value_type>
    >, hsize_t>::type
read_chunked_dataset(H5::DataSet const& dataset, T& data, ssize_t index)
{
    typedef typename T::value_type value_type;

    // determine extent of data space and resize result vector (if necessary)
    H5::DataSpace dataspace(dataset.getSpace());
    if (!has_rank<2>(dataspace)) {
        throw std::runtime_error("HDF5 reader: dataset has incompatible dataspace");
    }
    hsize_t dim[2];
    dataspace.getSimpleExtentDims(dim);
    data.resize(dim[1]);

    return detail::read_chunked_dataset<value_type, 1>(dataset, &*data.begin(), index);
}

//
// chunks of vector containers holding fixed-size arrays
//
// pass length of vector as third parameter
template <typename T>
inline typename boost::enable_if<boost::mpl::and_<
        is_vector<T>, is_array<typename T::value_type>
    >, H5::DataSet>::type
create_chunked_dataset(
    H5::CommonFG const& fg
  , std::string const& name
  , typename T::size_type size
  , hsize_t max_size=H5S_UNLIMITED)
{
    typedef typename T::value_type array_type;
    typedef typename array_type::value_type value_type;
    hsize_t shape[2] = { size, array_type::static_size };
    return detail::create_chunked_dataset<value_type, 2>(fg, name, shape, max_size);
}

template <typename T>
inline typename boost::enable_if<boost::mpl::and_<
        is_vector<T>, is_array<typename T::value_type>
    >, void>::type
write_chunked_dataset(H5::DataSet const& dataset, T const& data, hsize_t index=H5S_UNLIMITED)
{
    typedef typename T::value_type array_type;
    typedef typename array_type::value_type value_type;

    // assert data.size() corresponds to dataspace extents
    if (has_rank<3>(dataset)) {
        hsize_t dim[3];
        dataset.getSpace().getSimpleExtentDims(dim);
        if (data.size() != dim[1]) {
            throw std::runtime_error("HDF5 writer: dataset has incompatible dataspace");
        }
    }

    // raw data are laid out contiguously
    detail::write_chunked_dataset<value_type, 2>(dataset, &*data.begin()->begin(), index);
}

/** read chunk of vector container with array data, resize/reshape result array if necessary */
template <typename T>
inline typename boost::enable_if<boost::mpl::and_<
        is_vector<T>, is_array<typename T::value_type>
    >, hsize_t>::type
read_chunked_dataset(H5::DataSet const& dataset, T& data, ssize_t index)
{
    typedef typename T::value_type array_type;
    typedef typename array_type::value_type value_type;

    // determine extent of data space and resize result vector (if necessary)
    H5::DataSpace dataspace(dataset.getSpace());
    if (!has_rank<3>(dataspace)) {
        throw std::runtime_error("HDF5 reader: dataset has incompatible dataspace");
    }
    hsize_t dim[3];
    dataspace.getSimpleExtentDims(dim);
    data.resize(dim[1]);

    // raw data are laid out contiguously
    return detail::read_chunked_dataset<value_type, 2>(dataset, &*data.begin()->begin(), index);
}

} // namespace h5xx

#endif /* ! H5XX_CHUNKED_DATASET_HPP */
