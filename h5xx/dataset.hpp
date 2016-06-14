/*
 * Copyright © 2008-2011  Peter Colberg and Felix Höfling
 * All rights reserved.
 *
 * This file is part of h5xx — a C++ wrapper for the HDF5 library.
 *
 * This software may be modified and distributed under the terms of the
 * 3-clause BSD license.  See accompanying file LICENSE for details.
 */

#ifndef H5XX_DATASET_HPP
#define H5XX_DATASET_HPP

#include <h5xx/attribute.hpp>
#include <h5xx/property.hpp>
#include <h5xx/utility.hpp>

#include <boost/array.hpp>
#include <boost/mpl/and.hpp>
#include <boost/multi_array.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

#include <vector>

namespace h5xx {

namespace detail {

/**
 * Create dataset 'name' in given group/file. The dataset contains
 * a single entry only and should be written via write_dataset().
 *
 * This function creates missing intermediate groups.
 */
// generic case: some fundamental type and a shape of arbitrary rank
template <typename T, int rank>
inline typename boost::enable_if<boost::is_fundamental<T>, H5::DataSet>::type
create_dataset(
    H5::CommonFG const& fg
  , std::string const& name
  , hsize_t const* shape)
{
    H5::IdComponent const& loc = dynamic_cast<H5::IdComponent const&>(fg);

    // file dataspace holding a single multi_array of fixed rank
    H5::DataSpace dataspace(rank, shape);
    H5::DSetCreatPropList cparms;
    if (rank > 0 && sizeof(T) * shape[0] > 64) { // enable GZIP compression for at least 64 bytes
        cparms.setChunk(rank, shape);
        cparms.setDeflate(compression_level);
    }

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
 * write data to dataset
 */
// generic case: some fundamental type and a pointer to the contiguous array of data
// size and shape are taken from the dataset
template <typename T, int rank>
inline typename boost::enable_if<boost::is_fundamental<T>, void>::type
write_dataset(H5::DataSet const& dataset, T const* data)
{
    H5::DataSpace dataspace(dataset.getSpace());
    if (!has_rank<rank>(dataspace)) {
        throw std::runtime_error("HDF5 writer: dataset has incompatible dataspace");
    }
    dataset.write(data, ctype<T>::hid(), dataspace, dataspace);
}

/**
 * read data from dataset
 */
// generic case: some (fundamental) type and a pointer to the contiguous array of data
// size and shape are taken from the dataset
template <typename T, int rank>
inline typename boost::enable_if<boost::is_fundamental<T>, void>::type
read_dataset(H5::DataSet const& dataset, T* data)
{
    H5::DataSpace dataspace(dataset.getSpace());
    if (!has_rank<rank>(dataspace)) {
        throw std::runtime_error("HDF5 reader: dataset has incompatible dataspace");
    }
    try {
        H5XX_NO_AUTO_PRINT(H5::Exception);
        dataset.read(data, ctype<T>::hid(), dataspace, dataspace);
    }
    catch (H5::Exception const&) {
        throw std::runtime_error("HDF5 reader: failed to read multidimensional array data");
    }
}

} // namespace detail

//
// scalar/fundamental types
//
template <typename T>
inline typename boost::enable_if<boost::is_fundamental<T>, H5::DataSet>::type
create_dataset(
    H5::CommonFG const& fg
  , std::string const& name)
{
    return detail::create_dataset<T, 0>(fg, name, NULL);
}

template <typename T>
inline typename boost::enable_if<boost::is_fundamental<T>, void>::type
write_dataset(H5::DataSet const& dataset, T const& data)
{
    detail::write_dataset<T, 0>(dataset, &data);
}

template <typename T>
inline typename boost::enable_if<boost::is_fundamental<T>, void>::type
read_dataset(H5::DataSet const& dataset, T& data)
{
    return detail::read_dataset<T, 0>(dataset, &data);
}

//
// fixed-size arrays
//
template <typename T>
inline typename boost::enable_if<boost::mpl::and_<
        is_array<T>, boost::is_fundamental<typename T::value_type>
    >, H5::DataSet>::type
create_dataset(
    H5::CommonFG const& fg
  , std::string const& name)
{
    typedef typename T::value_type value_type;
    enum { rank = 1 };
    hsize_t shape[1] = { T::static_size };
    return detail::create_dataset<value_type, rank>(fg, name, shape);
}

template <typename T>
inline typename boost::enable_if<boost::mpl::and_<
        is_array<T>, boost::is_fundamental<typename T::value_type>
    >, void>::type
write_dataset(H5::DataSet const& dataset, T const& data)
{
    typedef typename T::value_type value_type;
    enum { rank = 1 };
    if (!has_extent<T>(dataset))
    {
        throw std::runtime_error("HDF5 writer: dataset has incompatible dataspace");
    }
    detail::write_dataset<value_type, rank>(dataset, &*data.begin());
}

template <typename T>
inline typename boost::enable_if<boost::mpl::and_<
        is_array<T>, boost::is_fundamental<typename T::value_type>
    >, void>::type
read_dataset(H5::DataSet const& dataset, T& data)
{
    typedef typename T::value_type value_type;
    enum { rank = 1 };
    detail::read_dataset<value_type, rank>(dataset, &*data.begin());
}

//
// multi-arrays of fixed rank
//
template <typename T>
inline typename boost::enable_if<is_multi_array<T>, H5::DataSet>::type
create_dataset(
    H5::CommonFG const& fg
  , std::string const& name
  , typename T::size_type const* shape)
{
    typedef typename T::element value_type;
    enum { rank = T::dimensionality };
    // convert T::size_type to hsize_t
    boost::array<hsize_t, rank> shape_;
    std::copy(shape, shape + rank, shape_.begin());
    return detail::create_dataset<value_type, rank>(fg, name, &*shape_.begin());
}

template <typename T>
inline typename boost::enable_if<is_multi_array<T>, void>::type
write_dataset(H5::DataSet const& dataset, T const& data)
{
    typedef typename T::element value_type;
    enum { rank = T::dimensionality };
    if (!has_extent<T>(dataset, data.shape()))
    {
        throw std::runtime_error("HDF5 writer: dataset has incompatible dataspace");
    }
    detail::write_dataset<value_type, rank>(dataset, data.origin());
}

/** read multi_array data, resize/reshape result array if necessary */
template <typename T>
inline typename boost::enable_if<is_multi_array<T>, void>::type
read_dataset(H5::DataSet const& dataset, T& data)
{
    typedef typename T::element value_type;
    enum { rank = T::dimensionality };

    // determine extent of data space
    H5::DataSpace dataspace(dataset.getSpace());
    if (!has_rank<rank>(dataspace)) {
        throw std::runtime_error("HDF5 reader: dataset has incompatible dataspace");
    }
    boost::array<hsize_t, rank> dim;
    dataspace.getSimpleExtentDims(&*dim.begin());

    // resize result array if necessary, may allocate new memory
    if (!std::equal(dim.begin(), dim.end(), data.shape())) {
        data.resize(dim);
    }

    return detail::read_dataset<value_type, rank>(dataset, data.origin());
}

//
// vector containers holding scalars
//
// pass length of vector as third parameter
template <typename T>
inline typename boost::enable_if<boost::mpl::and_<
        is_vector<T>, boost::is_fundamental<typename T::value_type>
    >, H5::DataSet>::type
create_dataset(
    H5::CommonFG const& fg
  , std::string const& name
  , typename T::size_type size)
{
    typedef typename T::value_type value_type;
    hsize_t shape[1] = { size };
    return detail::create_dataset<value_type, 1>(fg, name, shape);
}

template <typename T>
inline typename boost::enable_if<boost::mpl::and_<
        is_vector<T>, boost::is_fundamental<typename T::value_type>
    >, void>::type
write_dataset(H5::DataSet const& dataset, T const& data)
{
    typedef typename T::value_type value_type;

    // assert data.size() corresponds to dataspace extents
    if (has_rank<1>(dataset)) {
        hsize_t dim;
        dataset.getSpace().getSimpleExtentDims(&dim);
        if (data.size() != dim) {
            throw std::runtime_error("HDF5 writer: dataset has incompatible dataspace");
        }
    }

    detail::write_dataset<value_type, 1>(dataset, &*data.begin());
}

/** read vector container with scalar data, resize/reshape result array if necessary */
template <typename T>
inline typename boost::enable_if<boost::mpl::and_<
        is_vector<T>, boost::is_fundamental<typename T::value_type>
    >, void>::type
read_dataset(H5::DataSet const& dataset, T& data)
{
    typedef typename T::value_type value_type;

    // determine extent of data space and resize result vector (if necessary)
    H5::DataSpace dataspace(dataset.getSpace());
    if (!has_rank<1>(dataspace)) {
        throw std::runtime_error("HDF5 reader: dataset has incompatible dataspace");
    }
    hsize_t dim;
    dataspace.getSimpleExtentDims(&dim);
    data.resize(dim);

    detail::read_dataset<value_type, 1>(dataset, &*data.begin());
}

//
// vector containers holding fixed-size arrays
//
// pass length of vector as third parameter
template <typename T>
inline typename boost::enable_if<boost::mpl::and_<
        is_vector<T>, is_array<typename T::value_type>
    >, H5::DataSet>::type
create_dataset(
    H5::CommonFG const& fg
  , std::string const& name
  , typename T::size_type size)
{
    typedef typename T::value_type array_type;
    typedef typename array_type::value_type value_type;
    hsize_t shape[2] = { size, array_type::static_size };
    return detail::create_dataset<value_type, 2>(fg, name, shape);
}

template <typename T>
inline typename boost::enable_if<boost::mpl::and_<
        is_vector<T>, is_array<typename T::value_type>
    >, void>::type
write_dataset(H5::DataSet const& dataset, T const& data)
{
    typedef typename T::value_type array_type;
    typedef typename array_type::value_type value_type;

    // assert data.size() corresponds to dataspace extents
    if (has_rank<2>(dataset)) {
        hsize_t dim[2];
        dataset.getSpace().getSimpleExtentDims(dim);
        if (data.size() != dim[0]) {
            throw std::runtime_error("HDF5 writer: dataset has incompatible dataspace");
        }
    }

    // raw data are laid out contiguously
    detail::write_dataset<value_type, 2>(dataset, &*data.begin()->begin());
}

/** read vector container with array data, resize/reshape result array if necessary */
template <typename T>
inline typename boost::enable_if<boost::mpl::and_<
        is_vector<T>, is_array<typename T::value_type>
    >, void>::type
read_dataset(H5::DataSet const& dataset, T& data)
{
    typedef typename T::value_type array_type;
    typedef typename array_type::value_type value_type;

    // determine extent of data space and resize result vector (if necessary)
    H5::DataSpace dataspace(dataset.getSpace());
    if (!has_rank<2>(dataspace)) {
        throw std::runtime_error("HDF5 reader: dataset has incompatible dataspace");
    }
    hsize_t dim[2];
    dataspace.getSimpleExtentDims(dim);
    data.resize(dim[0]);

    // raw data are laid out contiguously
    detail::read_dataset<value_type, 2>(dataset, &*data.begin()->begin());
}

/**
 * Helper function to create a dataset on the fly and write to it.
 */
template <typename T>
inline void write_dataset(H5::CommonFG const& fg, std::string const& name, T const& data)
{
    H5::DataSet dataset = create_dataset<T>(fg, name);
    write_dataset(dataset, data);
}

/**
 * Helper function to open a dataset on the fly and read from it.
 */
template <typename T>
inline void read_dataset(H5::CommonFG const& fg, std::string const& name, T& data)
{
    H5E_BEGIN_TRY {
        // open dataset in file or group
        H5::IdComponent const& loc(dynamic_cast<H5::IdComponent const&>(fg));
        hid_t hid = H5Dopen(loc.getId(), name.c_str(), H5P_DEFAULT);
        if (hid > 0) {
            read_dataset(hid, data);
            H5Dclose(hid);
        }
        else {
            throw error("attempt to read non-existent dataset \"" + name + "\"");
        }
    } H5E_END_TRY
}

} // namespace h5xx

#endif /* ! H5XX_DATASET_HPP */
