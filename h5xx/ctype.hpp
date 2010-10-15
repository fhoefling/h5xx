/* HDF5 C++ extensions
 *
 * Copyright © 2008-2010  Peter Colberg and Felix Höfling
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

#ifndef H5XX_CTYPE_HPP
#define H5XX_CTYPE_HPP

#include <boost/array.hpp>
#include <boost/multi_array.hpp>
#include <boost/type_traits.hpp>

#include <h5xx/hdf5.hpp>

namespace H5
{

/*
 * fundamental type to HDF5 native data type translation
 */
template <typename T>
struct ctype;

#define MAKE_CTYPE(CTYPE, H5TYPE) \
template <> struct ctype<CTYPE> \
{ operator H5::PredType const& () { return H5::PredType::NATIVE_##H5TYPE;} }

MAKE_CTYPE(float, FLOAT);
MAKE_CTYPE(double, DOUBLE);
MAKE_CTYPE(long double, LDOUBLE);
MAKE_CTYPE(int8_t, INT8);
MAKE_CTYPE(uint8_t, UINT8);
MAKE_CTYPE(int16_t, INT16);
MAKE_CTYPE(uint16_t, UINT16);
MAKE_CTYPE(int32_t, INT32);
MAKE_CTYPE(uint32_t, UINT32);
MAKE_CTYPE(int64_t, INT64);
MAKE_CTYPE(uint64_t, UINT64);

#undef MAKE_CTYPE

template <typename T, typename U=typename T::value_type, size_t size=T::static_size>
struct is_boost_array : public boost::is_base_of<boost::array<U, size>, T> {};

template <typename T, typename U=typename T::element, size_t rank=T::dimensionality>
struct is_boost_multi_array : public boost::is_base_of<boost::multi_array<U, rank>, T> {};

template <typename T>
struct is_vector : public boost::false_type {};

template <typename T>
struct is_vector<std::vector<T> >: public boost::true_type {};

/**
 * check data type of abstract dataset (dataset or attribute)
 */
template <typename T>
typename boost::enable_if<boost::is_fundamental<T>, bool>::type
has_type(H5::AbstractDs const& ds)
{
    return ds.getDataType() == ctype<T>();
}

template <typename T>
typename boost::enable_if<boost::is_same<T, std::string>, bool>::type
has_type(H5::AbstractDs const& ds)
{
    return ds.getTypeClass() == H5T_STRING;
}

template <typename T>
typename boost::enable_if<boost::is_same<T, char const*>, bool>::type
has_type(H5::AbstractDs const& ds)
{
    return has_type<std::string>(ds);
}

template <typename T>
typename boost::enable_if<is_vector<T>, bool>::type
has_type(H5::AbstractDs const& ds)
{
    return has_type<typename T::value_type>(ds);
}

template <typename T>
typename boost::enable_if<is_boost_array<T>, bool>::type
has_type(H5::AbstractDs const& ds)
{
    return has_type<typename T::value_type>(ds);
}

template <typename T>
typename boost::enable_if<is_boost_multi_array<T>, bool>::type
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
bool has_rank(H5::DataSpace const& ds)
{
    return ds.isSimple() && ds.getSimpleExtentNdims() == rank;
}

/**
 * check data space rank of abstract dataset (dataset or attribute)
 */
template <hsize_t rank>
bool has_rank(H5::AbstractDs const& ds)
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
typename boost::enable_if<is_boost_array<T>, bool>::type
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
typename boost::enable_if<is_boost_multi_array<T>, bool>::type
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
typename boost::enable_if<is_boost_array<T>, bool>::type
has_extent(H5::DataSpace const& dataspace)
{
    return has_extent<T, 0>(dataspace);
}

template <typename T>
typename boost::enable_if<is_boost_multi_array<T>, bool>::type
has_extent(H5::DataSpace const& dataspace, typename T::size_type const* shape)
{
    return has_extent<T, 0>(dataspace);
}

/**
 * check data space extent of an H5::DataSet or H5::Attribute
 */
template <typename T, hsize_t extra_rank>
bool has_extent(H5::AbstractDs const& ds)
{
    return has_extent<T, extra_rank>(ds.getSpace());
}

template <typename T, hsize_t extra_rank>
bool has_extent(H5::AbstractDs const& ds, typename T::size_type const* shape)
{
    return has_extent<T, extra_rank>(ds.getSpace(), shape);
}

template <typename T>
bool has_extent(H5::AbstractDs const& ds)
{
    return has_extent<T, 0>(ds.getSpace());
}

template <typename T>
bool has_extent(H5::AbstractDs const& ds, typename T::size_type const* shape)
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

} // namespace H5

#endif /* ! H5XX_CTYPE_HPP */
