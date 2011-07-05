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

#ifndef H5XX_ATTRIBUTE_HPP
#define H5XX_ATTRIBUTE_HPP

#include <boost/any.hpp>
#include <boost/array.hpp>
#include <boost/mpl/and.hpp>
#include <boost/multi_array.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <vector>

#include <h5xx/ctype.hpp>
#include <h5xx/error.hpp>
#include <h5xx/exception.hpp>
#include <h5xx/utility.hpp>

namespace h5xx {

/*
 * create and write fundamental type attribute
 */
template <typename T>
typename boost::enable_if<boost::is_fundamental<T>, void>::type
write_attribute(H5::H5Object const& object, std::string const& name, T const& value)
{
    H5::Attribute attr;
    try {
        H5XX_NO_AUTO_PRINT(H5::AttributeIException);
        attr = object.openAttribute(name);
        if (!has_type<T>(attr) || !has_scalar_space(attr)) {
            // recreate attribute with proper type
            object.removeAttr(name);
            throw H5::AttributeIException();
        }
    }
    catch (H5::AttributeIException const&) {
        attr = object.createAttribute(name, ctype<T>::hid(), H5S_SCALAR);
    }
    attr.write(ctype<T>::hid(), &value);
}

/**
 * read fundamental type attribute
 */
template <typename T>
typename boost::enable_if<boost::is_fundamental<T>, T>::type
read_attribute(H5::H5Object const& object, std::string const& name)
{
    H5::Attribute attr;
    try {
        H5XX_NO_AUTO_PRINT(H5::AttributeIException);
        attr = object.openAttribute(name);
    }
    catch (H5::AttributeIException const&) {
        throw;
    }
    if (!has_scalar_space(attr)) {
        throw H5::AttributeIException("H5::attribute::as", "incompatible dataspace");
    }
    T value;
    attr.read(ctype<T>::hid(), &value);
    return value;
}

/**
 * create and write string attribute
 */
template <typename T>
typename boost::enable_if<boost::is_same<T, std::string>, void>::type
write_attribute(H5::H5Object const& object, std::string const& name, T const& value)
{
    H5::StrType tid(H5::PredType::C_S1, value.size());
    // remove attribute if it exists
    try {
        H5XX_NO_AUTO_PRINT(H5::AttributeIException);
        object.removeAttr(name);
    }
    catch (H5::AttributeIException const&) {}
    H5::Attribute attr = object.createAttribute(name, tid, H5S_SCALAR);
    attr.write(tid, value.data());
}

/**
 * read string attribute
 */
template <typename T>
typename boost::enable_if<boost::is_same<T, std::string>, T>::type
read_attribute(H5::H5Object const& object, std::string const& name)
{
    H5::Attribute attr;
    try {
        H5XX_NO_AUTO_PRINT(H5::AttributeIException);
        attr = object.openAttribute(name);
    }
    catch (H5::AttributeIException const&) {
        throw;
    }
    if (!has_scalar_space(attr)) {
        throw H5::AttributeIException("H5::attribute::as", "incompatible dataspace");
    }
    // determine string length first and allocate space
    size_t len = attr.getDataType().getSize();
    std::vector<std::string::value_type> value(len);
    attr.read(H5::StrType(H5::PredType::C_S1, len), &*value.begin());
    return std::string(&*value.begin(), value.size());
}

/**
 * create and write C string attribute
 */
template <typename T>
typename boost::enable_if<boost::is_same<T, char const*>, void>::type
write_attribute(H5::H5Object const& object, std::string const& name, T value)
{
    H5::StrType tid(H5::PredType::C_S1, strlen(value));
    // remove attribute if it exists
    try {
        H5XX_NO_AUTO_PRINT(H5::AttributeIException);
        object.removeAttr(name);
    }
    catch (H5::AttributeIException const&) {}
    H5::Attribute attr = object.createAttribute(name, tid, H5S_SCALAR);
    attr.write(tid, value);
}

/*
 * create and write fixed-size array type attribute
 */
template <typename T>
typename boost::enable_if<boost::mpl::and_<is_array<T>, boost::is_fundamental<typename T::value_type> >, void>::type
write_attribute(H5::H5Object const& object, std::string const& name, T const& value)
{
    typedef typename T::value_type value_type;
    enum { size = T::static_size };

    H5::Attribute attr;
    try {
        H5XX_NO_AUTO_PRINT(H5::AttributeIException);
        attr = object.openAttribute(name);
        if (!has_type<T>(attr) || !has_extent<T>(attr)) {
            // recreate attribute with proper type and size
            object.removeAttr(name);
            throw H5::AttributeIException();
        }
    }
    catch (H5::AttributeIException const&) {
        hsize_t dim[1] = { size };
        H5::DataSpace ds(1, dim);
        attr = object.createAttribute(name, ctype<value_type>::hid(), ds);
    }
    attr.write(ctype<value_type>::hid(), &value.front());
}

/*
 * create and write fixed-size C string array type attribute
 */
template <typename T>
typename boost::enable_if<boost::mpl::and_<is_array<T>, boost::is_same<typename T::value_type, char const*> >, void>::type
write_attribute(H5::H5Object const& object, std::string const& name, T const& value)
{
    typedef typename T::value_type value_type;
    enum { size = T::static_size };

    hsize_t dim[1] = { size };
    H5::DataSpace ds(1, dim);
    size_t max_len = 0;
    for (size_t i = 0; i < size; ++i) {
        max_len = std::max(max_len, strlen(value[i]) + 1);  // include terminating NULL character
    }
    H5::StrType tid(H5::PredType::C_S1, max_len);
    // remove attribute if it exists
    try {
        H5XX_NO_AUTO_PRINT(H5::AttributeIException);
        object.removeAttr(name);
    }
    catch (H5::AttributeIException const&) {}
    H5::Attribute attr = object.createAttribute(name, tid, ds);
    std::vector<char> data(max_len * size);
    for (size_t i = 0; i < size; ++i) {
        strncpy(&data.front() + i * max_len, value[i], max_len);
    }
    attr.write(tid, &data.front());
}

/**
 * read fixed-size array type attribute
 */
template <typename T>
typename boost::enable_if<boost::mpl::and_<is_array<T>, boost::is_fundamental<typename T::value_type> >, T>::type
read_attribute(H5::H5Object const& object, std::string const& name)
{
    typedef typename T::value_type value_type;
    enum { size = T::static_size };

    H5::Attribute attr;
    try {
        H5XX_NO_AUTO_PRINT(H5::AttributeIException);
        attr = object.openAttribute(name);
    }
    catch (H5::AttributeIException const&) {
        throw;
    }

    if (!has_extent<T>(attr)) {
        throw H5::AttributeIException("H5::attribute::as", "incompatible dataspace");
    }

    T value;
    attr.read(ctype<value_type>::hid(), &value.front());
    return value;
}

/*
 * create and write multi-dimensional array type attribute
 */
template <typename T>
typename boost::enable_if<is_multi_array<T>, void>::type
write_attribute(H5::H5Object const& object, std::string const& name, T const& value)
{
    typedef typename T::element value_type;
    enum { rank = T::dimensionality };

    H5::Attribute attr;
    try {
        H5XX_NO_AUTO_PRINT(H5::AttributeIException);
        attr = object.openAttribute(name);
        if (!has_type<T>(attr) || !has_extent<T>(attr, value.shape())) {
            // recreate attribute with proper type and size
            object.removeAttr(name);
            throw H5::AttributeIException();
        }
    }
    catch (H5::AttributeIException const&) {
        hsize_t dim[rank];
        std::copy(value.shape(), value.shape() + rank, dim);
        H5::DataSpace ds(rank, dim);
        attr = object.createAttribute(name, ctype<value_type>::hid(), ds);
    }
    attr.write(ctype<value_type>::hid(), value.origin());
}

/**
 * read multi-dimensional array type attribute
 */
template <typename T>
typename boost::enable_if<is_multi_array<T>, T>::type
read_attribute(H5::H5Object const& object, std::string const& name)
{
    typedef typename T::element value_type;
    enum { rank = T::dimensionality };

    H5::Attribute attr;
    try {
        H5XX_NO_AUTO_PRINT(H5::AttributeIException);
        attr = object.openAttribute(name);
    }
    catch (H5::AttributeIException const&) {
        throw;
    }

    H5::DataSpace ds(attr.getSpace());
    if (!has_rank<rank>(attr)) {
        throw H5::AttributeIException("H5::attribute::as", "incompatible dataspace");
    }

    hsize_t dim[rank];
    ds.getSimpleExtentDims(dim);
    boost::array<size_t, rank> shape;
    std::copy(dim, dim + rank, shape.begin());
    boost::multi_array<value_type, rank> value(shape);
    attr.read(ctype<value_type>::hid(), value.origin());
    return value;
}

/*
 * create and write vector type attribute
 */
template <typename T>
typename boost::enable_if<is_vector<T>, void>::type
write_attribute(H5::H5Object const& object, std::string const& name, T const& value)
{
    typedef typename T::value_type value_type;

    H5::Attribute attr;
    try {
        H5XX_NO_AUTO_PRINT(H5::AttributeIException);
        attr = object.openAttribute(name);
        if (!has_type<T>(attr) || elements(attr) != value.size()) {
            // recreate attribute with proper type
            object.removeAttr(name);
            throw H5::AttributeIException();
        }
    }
    catch (H5::AttributeIException const&) {
        hsize_t dim[1] = { value.size() };
        H5::DataSpace ds(1, dim);
        attr = object.createAttribute(name, ctype<value_type>::hid(), ds);
    }
    attr.write(ctype<value_type>::hid(), &*value.begin());
}

/**
 * read vector type attribute
 *
 * read data of possibly higher rank into 1D std::vector
 */
template <typename T>
typename boost::enable_if<is_vector<T>, T>::type
read_attribute(H5::H5Object const& object, std::string const& name)
{
    typedef typename T::value_type value_type;

    H5::Attribute attr;
    try {
        H5XX_NO_AUTO_PRINT(H5::AttributeIException);
        attr = object.openAttribute(name);
    }
    catch (H5::AttributeIException const&) {
        throw;
    }

    H5::DataSpace ds(attr.getSpace());
    if (!ds.isSimple()) {
        throw H5::AttributeIException("H5::attribute::as", "incompatible dataspace");
    }
    size_t size = ds.getSimpleExtentNpoints();
    std::vector<value_type> value(size);
    attr.read(ctype<value_type>::hid(), &*value.begin());
    return value;
}

/**
 * determine whether attribute exists in file/group/dataset
 */
inline bool exists_attribute(H5::H5Object const& object, std::string const& name)
{
    htri_t tri = H5Aexists(object.getId(), name.c_str());
    if (tri < 0) {
        throw error("failed to determine whether attribute \"" + name + "\" exists");
    }
    return (tri > 0);
}

/**
 * returns attribute value as boost::any if exists, or empty boost::any otherwise
 */
template <typename T>
boost::any read_attribute_if_exists(H5::H5Object const& object, std::string const& name)
{
    if (exists_attribute(object, name)) {
        return read_attribute<T>(object, name);
    }
    return boost::any();
}

} // namespace h5xx

#endif /* ! H5XX_ATTRIBUTE_HPP */
