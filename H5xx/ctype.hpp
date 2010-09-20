/* HDF5 C++ extensions
 *
 * Copyright © 2008-2009  Peter Colberg and Felix Höfling
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

#ifndef HALMD_UTIL_H5XX_CTYPE_HPP
#define HALMD_UTIL_H5XX_CTYPE_HPP

#define H5E_auto_t_vers 2
#include <H5Cpp.h>

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

template <typename T>
struct is_boost_array : public boost::false_type {};

template <typename T, size_t size>
struct is_boost_array<boost::array<T, size> >: public boost::true_type {};

template <typename T>
struct is_boost_multi_array : public boost::false_type {};

template <typename T, size_t dimension>
struct is_boost_multi_array<boost::multi_array<T, dimension> >: public boost::true_type {};

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
 * the bool parameter needs to be false for datasets which contain
 * multiple values (enumerated by the first dimension)
 */
template <typename T>
typename boost::enable_if<is_boost_array<T>, bool>::type
has_extent(H5::DataSpace const& dataspace, bool is_single_valued=true)
{
    if (is_single_valued && has_rank<1>(dataspace)) {
        hsize_t dim[1];
        dataspace.getSimpleExtentDims(dim);
        return dim[0] == T::static_size;
    }
    else if(!is_single_valued && has_rank<2>(dataspace)) {
        hsize_t dim[2];
        dataspace.getSimpleExtentDims(dim);
        return dim[1] == T::static_size;
    }
    else
        return false;
}

template <typename T>
typename boost::enable_if<is_boost_multi_array<T>, bool>::type
has_extent(H5::DataSpace const& dataspace, typename T::size_type const* shape, bool is_single_valued=true)
{
    enum { rank = T::dimensionality };
    if (is_single_valued && has_rank<rank>(dataspace)) {
        boost::array<hsize_t, rank> dim;
        dataspace.getSimpleExtentDims(dim.data());
        return std::equal(dim.begin(), dim.end(), shape);
    }
    else if (!is_single_valued && has_rank<rank+1>(dataspace)) {
        boost::array<hsize_t, rank+1> dim;
        dataspace.getSimpleExtentDims(dim.data());
        return std::equal(dim.begin()+1, dim.end(), shape);
    }
    else
        return false;
}

/**
 * check data space extent of an H5::DataSet or H5::Attribute
 */
template <typename T>
bool has_extent(H5::AbstractDs const& ds, bool is_single_valued=true)
{
    return has_extent<T>(ds.getSpace(), is_single_valued);
}

template <typename T>
bool has_extent(H5::AbstractDs const& ds, typename T::size_type const* shape, bool is_single_valued=true)
{
    return has_extent<T>(ds.getSpace(), shape, is_single_valued);
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

#endif /* ! HALMD_UTIL_H5XX_CTYPE_HPP */
