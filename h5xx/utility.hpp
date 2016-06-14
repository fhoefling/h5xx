/*
 * Copyright © 2008-2011 Felix Höfling
 * Copyright © 2008-2012 Peter Colberg
 * Copyright © 2015      Nicolas Höft
 * All rights reserved.
 *
 * This file is part of h5xx — a C++ wrapper for the HDF5 library.
 *
 * This software may be modified and distributed under the terms of the
 * 3-clause BSD license.  See accompanying file LICENSE for details.
 */

#ifndef H5XX_UTILITY_HPP
#define H5XX_UTILITY_HPP

#include <h5xx/ctype.hpp>
#include <h5xx/error.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/array.hpp>
#include <boost/multi_array.hpp>
#include <boost/type_traits/is_fundamental.hpp>
#include <boost/type_traits/is_same.hpp>

#include <string>
#include <list>

namespace h5xx {

using h5xx::detail::ctype; // FIXME HDF5 C++ to C transition

/**
 * returns absolute path of an HDF5 object within file
 *
 * For attributes the path of the parent object is returned.
 */
inline std::string path(H5::IdComponent const& id)
{
    ssize_t size; // excludes NULL terminator
    if (-1 == (size = H5Iget_name(id.getId(), NULL, 0))) {
        throw H5::IdComponentException("H5Iget_name", "failed to get length of name");
    }
    std::vector<char> name_(size + 1); // includes NULL terminator
    if (-1 == (H5Iget_name(id.getId(), &*name_.begin(), name_.size()))) {
        throw H5::IdComponentException("H5Iget_name", "failed to get name");
    }
    return &*name_.begin();
}

/**
 * split path_string on '/' and return list of group names,
 * empty names are suppressed
 */
inline std::list<std::string> split_path(std::string const& path_string)
{
    using namespace std;
    using namespace boost::algorithm;

    list<string> groups;
    split(groups, path_string, is_any_of("/"));  // equal('/')
    // drop empty strings (if path starts or ends with '/')
    for (list<string>::iterator s=groups.begin(); s != groups.end(); ) {
        if (*s == "") {
            groups.erase(s++);
        }
        else
            ++s;
    }

    return groups;
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

/**
 * return if a handle id is valid or not
 */
inline bool is_valid(hid_t hid)
{
#if H5_VERSION_LE(1,8,13)
    return hid > 0;  // the C++ API of HDF5 ≤ 1.8.13 may return 0 for invalid hids
#else
    return hid >= 0; // the above has been fixed in HDF5 1.8.14
#endif /* H5_VERSION_LE(1,8,13) */
}


} // namespace h5xx

#endif /* ! H5XX_UTILITY_HPP */
