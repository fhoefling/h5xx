/*
 * Copyright © 2008-2014 Felix Höfling
 * Copyright © 2014      Manuel Dibak
 * Copyright © 2008-2009 Peter Colberg
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

#ifndef H5XX_UTILITY_HPP
#define H5XX_UTILITY_HPP

#include <h5xx/ctype.hpp>
#include <h5xx/error.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/multi_array.hpp>
#include <boost/type_traits/is_fundamental.hpp>
#include <boost/type_traits/is_same.hpp>

#include <string>
#include <list>

namespace h5xx {

using h5xx::detail::ctype; // FIXME HDF5 C++ to C transition

/**
 * returns the associated file of an h5xx Object
 */
template <typename h5xxObject>
inline std::string filename(h5xxObject const& obj)
{
    hid_t hid = obj.hid();
    if (hid < 0) {
        throw error("h5xx::filename: object is empty");
    }
    ssize_t size = H5Fget_name(hid, NULL, 0);        // determine string length
    if (size < 0) {
        throw error("retrieving filename of HDF5 object with ID " + boost::lexical_cast<std::string>(hid));
    }
    std::vector<char> buffer(size + 1);
    H5Fget_name(hid, &*buffer.begin(), buffer.size()); // get string data
    return &*buffer.begin();
}

/**
 * Return the name (absolute path) of an h5xx object, or a general HDF5 object
 * given by its hid
 *
 * For attributes, return the name of the object to which that attribute is
 * attached.
 */
inline std::string get_name(hid_t hid)
{
    ssize_t size = H5Iget_name(hid, NULL, 0);        // get size of string
    if (size < 0) {
        throw error("failed to get name of HDF5 object with ID " + boost::lexical_cast<std::string>(hid));
    }
    std::vector<char> buffer;
    buffer.resize(size + 1);                         // includes NULL terminator
    size = H5Iget_name(hid, &*buffer.begin(), buffer.size()); // get string data
    return &*buffer.begin();                         // convert char* to std::string
}

template <typename h5xxObject>
inline std::string get_name(h5xxObject const& obj)
{
    return get_name(obj.hid());
}

/**
 * hard link HDF5 object into the given group with given name
 */
inline void link(H5::H5Object const& object, H5::Group const& group, std::string const& name)
{
    if (0 > H5Lcreate_hard(object.getId(), ".", group.getId(), name.c_str(), H5P_DEFAULT, H5P_DEFAULT)) {
        throw error("failed to link object");
    }
}

/**
 * determine whether dataset exists in file or group
 */
inline bool exists_dataset(H5::CommonFG const& fg, std::string const& name)
{
    H5::IdComponent const& loc(dynamic_cast<H5::IdComponent const&>(fg));
    hid_t hid;
    H5E_BEGIN_TRY {
        hid = H5Dopen(loc.getId(), name.c_str(), H5P_DEFAULT);
        if (hid > 0) {
            H5Dclose(hid);
        }
    } H5E_END_TRY
    return (hid > 0);
}

/**
 * Data type is a fixed-size RandomAccessCollection
 *
 * http://www.boost.org/doc/libs/release/libs/utility/Collection.html
 */
template <typename T>
struct is_array
  : boost::false_type {};

template <typename T, size_t size>
struct is_array<boost::array<T, size> >
  : boost::true_type {};

/**
 * Data type is a MultiArray
 *
 * http://www.boost.org/doc/libs/release/libs/multi_array/doc/reference.html#MultiArray
 */
template <typename T>
struct is_multi_array
  : boost::false_type {};

template <typename T, size_t size, typename Alloc>
struct is_multi_array<boost::multi_array<T, size, Alloc> >
  : boost::true_type {};

/**
 * Data type is a Random Access Container
 *
 * http://www.sgi.com/tech/stl/RandomAccessContainer.html
 */
template <typename T>
struct is_vector
  : boost::false_type {};

template <typename T, typename Alloc>
struct is_vector<std::vector<T, Alloc> >
  : boost::true_type {};


/**
 * h5xx implementation for checking data types of abstract datasets (dataset or attribute)
 */
template <typename T>
inline typename boost::enable_if<boost::is_fundamental<T>, bool>::type
has_type(hid_t const& hid)
{
    hid_t type_id = H5Aget_type(hid); // FIXME works for attributes only
    return H5Tget_class(type_id) == ctype<T>::hid();
}

template <typename T>
inline typename boost::enable_if<boost::is_same<T, std::string>, bool>::type
has_type(hid_t const& hid)
{
    hid_t type_id = H5Aget_type(hid);
    return H5Tget_class(type_id) == H5T_STRING;
}

template <typename T>
inline typename boost::enable_if<boost::is_same<T, char const*>, bool>::type
has_type(hid_t const& hid)
{
    return has_type<std::string>(hid);
}

template <typename T>
inline typename boost::enable_if<is_vector<T>, bool>::type
has_type(hid_t const& hid)
{
    return has_type<typename T::value_type>(hid);
}

template <typename T>
inline typename boost::enable_if<is_array<T>, bool>::type
has_type(hid_t const& hid)
{
    return has_type<typename T::value_type>(hid);
}

template <typename T>
inline typename boost::enable_if<is_multi_array<T>, bool>::type
has_type(hid_t const& hid)
{
    return has_type<typename T::element>(hid);
}

/**
 * check data type of abstract dataset (dataset or attribute)
 */

template <typename T>
inline typename boost::enable_if<boost::is_fundamental<T>, bool>::type
has_type(H5::AbstractDs const& ds)
{
    return ds.getDataType() == ctype<T>::hid();
}

template <typename T>
inline typename boost::enable_if<boost::is_same<T, std::string>, bool>::type
has_type(H5::AbstractDs const& ds)
{
    return ds.getTypeClass() == H5T_STRING;
}

template <typename T>
inline typename boost::enable_if<boost::is_same<T, char const*>, bool>::type
has_type(H5::AbstractDs const& ds)
{
    return has_type<std::string>(ds);
}

template <typename T>
inline typename boost::enable_if<is_vector<T>, bool>::type
has_type(H5::AbstractDs const& ds)
{
    return has_type<typename T::value_type>(ds);
}

template <typename T>
inline typename boost::enable_if<is_array<T>, bool>::type
has_type(H5::AbstractDs const& ds)
{
    return has_type<typename T::value_type>(ds);
}

template <typename T>
inline typename boost::enable_if<is_multi_array<T>, bool>::type
has_type(H5::AbstractDs const& ds)
{
    return has_type<typename T::element>(ds);
}

/**
 * check if data space is scalar
 */
inline bool is_scalar(H5::DataSpace const& dataspace)
{
    return dataspace.getSimpleExtentType() == H5S_SCALAR;
}

/**
 * check data space of abstract dataset (dataset or attribute)
 */
inline bool has_scalar_space(H5::AbstractDs const& ds)
{
    return is_scalar(ds.getSpace());
}

/**
 * check rank of dataspace given by its id
 */
template <hsize_t rank>
inline bool has_rank(hid_t space_id)
{
    return H5Sget_simple_extent_ndims(space_id) == rank;
}

/**
 * check rank of data space
 */
template <hsize_t rank>
inline bool has_rank(H5::DataSpace const& ds)
{
    return ds.isSimple() && ds.getSimpleExtentNdims() == rank;
}

/**
 * check data space rank of abstract dataset (dataset or attribute)
 */
template <hsize_t rank>
inline bool has_rank(H5::AbstractDs const& ds)
{
    return has_rank<rank>(ds.getSpace());
}

/**
 * check data space extent
 *
 * The parameter extra_rank specifies how many dimensions
 * are skipped. It should be 1 for multi-valued datasets
 * and 2 multi-valued datasets of std::vector.
 */
template <typename T>
inline typename boost::enable_if<is_array<T>, bool>::type
has_extent(hid_t space_id)
{
    if ( has_rank<1>(space_id) )
    {
        hsize_t dim[1];
        hsize_t maxdim[1];
        H5Sget_simple_extent_dims(space_id, dim, maxdim);
        return dim[0] == T::static_size;
    }
    else
        return false;

}

template <typename T, hsize_t extra_rank>
inline typename boost::enable_if<is_array<T>, bool>::type
has_extent(H5::DataSpace const& dataspace)
{
    // check extent of last dimension
    if (has_rank<1 + extra_rank>(dataspace)) {
        hsize_t dim[1 + extra_rank];
        dataspace.getSimpleExtentDims(dim);
        return dim[extra_rank] == T::static_size;
    }
    else
        return false;
}

template <typename T, hsize_t extra_rank>
inline typename boost::enable_if<is_multi_array<T>, bool>::type
has_extent(H5::DataSpace const& dataspace, typename T::size_type const* shape)
{
    enum { rank = T::dimensionality };
    if (has_rank<rank + extra_rank>(dataspace)) {
        boost::array<hsize_t, rank + extra_rank> dim;
        dataspace.getSimpleExtentDims(dim.data());
        return std::equal(dim.begin() + extra_rank, dim.end(), shape);
    }
    else
        return false;
}

template <typename T>
inline typename boost::enable_if<is_array<T>, bool>::type
has_extent(H5::DataSpace const& dataspace)
{
    return has_extent<T, 0>(dataspace);
}

template <typename T>
inline typename boost::enable_if<is_multi_array<T>, bool>::type
has_extent(H5::DataSpace const& dataspace, typename T::size_type const* shape)
{
    return has_extent<T, 0>(dataspace);
}

/**
 * check data space extent of an H5::DataSet or H5::Attribute
 */
template <typename T, hsize_t extra_rank>
inline bool has_extent(H5::AbstractDs const& ds)
{
    return has_extent<T, extra_rank>(ds.getSpace());
}

template <typename T, hsize_t extra_rank>
inline bool has_extent(H5::AbstractDs const& ds, typename T::size_type const* shape)
{
    return has_extent<T, extra_rank>(ds.getSpace(), shape);
}

template <typename T>
inline bool has_extent(H5::AbstractDs const& ds)
{
    return has_extent<T, 0>(ds.getSpace());
}

template <typename T>
inline bool has_extent(H5::AbstractDs const& ds, typename T::size_type const* shape)
{
    return has_extent<T, 0>(ds.getSpace(), shape);
}

/**
 * return total number of elements of a dataspace
 */
inline hsize_t elements(H5::DataSpace const& dataspace)
{
    return dataspace.getSimpleExtentNpoints();
}

/**
 * return total number of data elements in a dataset or attribute
 */
inline hsize_t elements(H5::AbstractDs const& ds)
{
    return elements(ds.getSpace());
}

/** swaps two h5xx objects */
template <typename h5xxObject>
void swap(h5xxObject& left, h5xxObject& right)
{
    hid_t tmp = left.hid_;
    left.hid_ = right.hid_;
    right.hid_ = tmp;
}

/**
 * Moves the object from 'right' to the returned temporary and leaves a default
 * constructed object in 'right'. Can be used to implement move semantics in
 * copying and assignment. Inspired by std::move in C++11.
 */
template <typename T>
T move(T& right)
{
    T left;
    swap(left, right);
    return left;
}

} // namespace h5xx

#endif /* ! H5XX_UTILITY_HPP */
