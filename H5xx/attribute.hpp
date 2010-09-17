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

#ifndef HALMD_UTIL_H5XX_ATTRIBUTE_HPP
#define HALMD_UTIL_H5XX_ATTRIBUTE_HPP

#define H5E_auto_t_vers 2
#include <H5Cpp.h>

#include <boost/array.hpp>
#include <boost/mpl/and.hpp>
#include <boost/multi_array.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <vector>

#include <H5xx/ctype.hpp>
#include <H5xx/exception.hpp>
#include <H5xx/util.hpp>

namespace H5xx
{

/**
 * HDF5 attribute
 */
class attribute
{
public:
    attribute(H5::H5Object const& node, std::string const& name)
        : m_node(&node), m_name(name) {}

    template <typename T>
    typename boost::enable_if<boost::is_fundamental<T>, attribute&>::type
    operator=(T const& value);
    template <typename T>
    typename boost::enable_if<boost::is_fundamental<T>, T>::type
    as();

    template <typename T>
    typename boost::enable_if<boost::is_same<T, std::string>, attribute&>::type
    operator=(T const& value);
    template <typename T>
    typename boost::enable_if<boost::is_same<T, std::string>, T>::type
    as();

    template <typename T>
    typename boost::enable_if<boost::is_same<T, char const*>, attribute&>::type
    operator=(T value);

    template <typename T>
    typename boost::enable_if<boost::mpl::and_<is_boost_array<T>, boost::is_fundamental<typename T::value_type> >, attribute&>::type
    operator=(T const& value);
    template <typename T>
    typename boost::enable_if<boost::mpl::and_<is_boost_array<T>, boost::is_fundamental<typename T::value_type> >, T>::type
    as();

    template <typename T>
    typename boost::enable_if<boost::mpl::and_<is_boost_array<T>, boost::is_same<typename T::value_type, char const*> >, attribute&>::type
    operator=(T const& value);

    template <typename T>
    typename boost::enable_if<is_boost_multi_array<T>, attribute&>::type
    operator=(T const& value);
    template <typename T>
    typename boost::enable_if<is_boost_multi_array<T>, T>::type
    as();

    template <typename T>
    typename boost::enable_if<is_vector<T>, attribute&>::type
    operator=(T const& value);
    template <typename T>
    typename boost::enable_if<is_vector<T>, T>::type
    as();

private:
    /** object which attribute belongs to */
    H5::H5Object const* m_node;
    /** attribute name */
    std::string m_name;
};

/*
 * create and write fundamental type attribute
 */
template <typename T>
typename boost::enable_if<boost::is_fundamental<T>, attribute&>::type
attribute::operator=(T const& value)
{
    H5::Attribute attr;
    try {
        H5XX_NO_AUTO_PRINT(H5::AttributeIException);
        attr = m_node->openAttribute(m_name);
        if (!has_type<T>(attr) || !has_scalar_space(attr)) {
            // recreate attribute with proper type
            m_node->removeAttr(m_name);
            throw H5::AttributeIException();
        }
    }
    catch (H5::AttributeIException const&) {
        attr = m_node->createAttribute(m_name, ctype<T>(), H5S_SCALAR);
    }
    attr.write(ctype<T>(), &value);
    return *this;
}

/**
 * read fundamental type attribute
 */
template <typename T>
typename boost::enable_if<boost::is_fundamental<T>, T>::type
attribute::as()
{
    H5::Attribute attr;
    try {
        H5XX_NO_AUTO_PRINT(H5::AttributeIException);
        attr = m_node->openAttribute(m_name);
    }
    catch (H5::AttributeIException const&) {
        throw;
    }
    if (!has_type<T>(attr) || !has_scalar_space(attr)) {
        throw H5::AttributeIException("H5xx::attribute::as", "incompatible dataspace");
    }
    T value;
    attr.read(ctype<T>(), &value);
    return value;
}

/**
 * create and write string attribute
 */
template <typename T>
typename boost::enable_if<boost::is_same<T, std::string>, attribute&>::type
attribute::operator=(T const& value)
{
    H5::StrType tid(H5::PredType::C_S1, value.size());
    // remove attribute if it exists
    try {
        H5XX_NO_AUTO_PRINT(H5::AttributeIException);
        m_node->removeAttr(m_name);
    }
    catch (H5::AttributeIException const&) {}
    H5::Attribute attr = m_node->createAttribute(m_name, tid, H5S_SCALAR);
    attr.write(tid, value.data());
    return *this;
}

/**
 * read string attribute
 */
template <typename T>
typename boost::enable_if<boost::is_same<T, std::string>, T>::type
attribute::as()
{
    H5::Attribute attr;
    try {
        H5XX_NO_AUTO_PRINT(H5::AttributeIException);
        attr = m_node->openAttribute(m_name);
    }
    catch (H5::AttributeIException const&) {
        throw;
    }
    if (!has_type<T>(attr) || !has_scalar_space(attr)) {
        throw H5::AttributeIException("H5xx::attribute::as", "incompatible dataspace");
    }
    // determine string length first and allocate space
    size_t len = attr.getDataType().getSize();
    std::vector<std::string::value_type> value(len);
    attr.read(H5::StrType(H5::PredType::C_S1, len), value.data());
    return std::string(value.data(), value.size());
}

/**
 * create and write C string attribute
 */
template <typename T>
typename boost::enable_if<boost::is_same<T, char const*>, attribute&>::type
attribute::operator=(T value)
{
    H5::StrType tid(H5::PredType::C_S1, strlen(value));
    // remove attribute if it exists
    try {
        H5XX_NO_AUTO_PRINT(H5::AttributeIException);
        m_node->removeAttr(m_name);
    }
    catch (H5::AttributeIException const&) {}
    H5::Attribute attr = m_node->createAttribute(m_name, tid, H5S_SCALAR);
    attr.write(tid, value);
    return *this;
}

/*
 * create and write fixed-size array type attribute
 */
template <typename T>
typename boost::enable_if<boost::mpl::and_<is_boost_array<T>, boost::is_fundamental<typename T::value_type> >, attribute&>::type
attribute::operator=(T const& value)
{
    typedef typename T::value_type value_type;
    enum { size = T::static_size };

    H5::Attribute attr;
    try {
        H5XX_NO_AUTO_PRINT(H5::AttributeIException);
        attr = m_node->openAttribute(m_name);
        if (!has_type<T>(attr) || !has_extent<T>(attr)) {
            // recreate attribute with proper type and size
            m_node->removeAttr(m_name);
            throw H5::AttributeIException();
        }
    }
    catch (H5::AttributeIException const&) {
        hsize_t dim[1] = { size };
        H5::DataSpace ds(1, dim);
        attr = m_node->createAttribute(m_name, ctype<value_type>(), ds);
    }
    attr.write(ctype<value_type>(), value.data());
    return *this;
}

/*
 * create and write fixed-size C string array type attribute
 */
template <typename T>
typename boost::enable_if<boost::mpl::and_<is_boost_array<T>, boost::is_same<typename T::value_type, char const*> >, attribute&>::type
attribute::operator=(T const& value)
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
        m_node->removeAttr(m_name);
    }
    catch (H5::AttributeIException const&) {}
    H5::Attribute attr = m_node->createAttribute(m_name, tid, ds);
    std::vector<char> data(max_len * size);
    for (size_t i = 0; i < size; ++i) {
        strncpy(data.data() + i * max_len, value[i], max_len);
    }
    attr.write(tid, data.data());
    return *this;
}

/**
 * read fixed-size array type attribute
 */
template <typename T>
typename boost::enable_if<boost::mpl::and_<is_boost_array<T>, boost::is_fundamental<typename T::value_type> >, T>::type
attribute::as()
{
    typedef typename T::value_type value_type;
    enum { size = T::static_size };

    H5::Attribute attr;
    try {
        H5XX_NO_AUTO_PRINT(H5::AttributeIException);
        attr = m_node->openAttribute(m_name);
    }
    catch (H5::AttributeIException const&) {
        throw;
    }

    if (!has_type<T>(attr) || !has_extent<T>(attr)) {
        throw H5::AttributeIException("H5xx::attribute::as", "incompatible dataspace");
    }

    boost::array<value_type, size> value;
    attr.read(ctype<value_type>(), value.data());
    return value;
}

/*
 * create and write multi-dimensional array type attribute
 */
template <typename T>
typename boost::enable_if<is_boost_multi_array<T>, attribute&>::type
attribute::operator=(T const& value)
{
    typedef typename T::element value_type;
    enum { rank = T::dimensionality };

    H5::Attribute attr;
    try {
        H5XX_NO_AUTO_PRINT(H5::AttributeIException);
        attr = m_node->openAttribute(m_name);
        if (!has_type<T>(attr) || !has_extent<T>(attr, value.shape())) {
            // recreate attribute with proper type and size
            m_node->removeAttr(m_name);
            throw H5::AttributeIException();
        }
    }
    catch (H5::AttributeIException const&) {
        hsize_t dim[rank];
        std::copy(value.shape(), value.shape() + rank, dim);
        H5::DataSpace ds(rank, dim);
        attr = m_node->createAttribute(m_name, ctype<value_type>(), ds);
    }
    attr.write(ctype<value_type>(), value.data());
    return *this;
}

/**
 * read multi-dimensional array type attribute
 */
template <typename T>
typename boost::enable_if<is_boost_multi_array<T>, T>::type
attribute::as()
{
    typedef typename T::element value_type;
    enum { rank = T::dimensionality };

    H5::Attribute attr;
    try {
        H5XX_NO_AUTO_PRINT(H5::AttributeIException);
        attr = m_node->openAttribute(m_name);
    }
    catch (H5::AttributeIException const&) {
        throw;
    }

    H5::DataSpace ds(attr.getSpace());
    if (!has_type<T>(attr) || !has_rank<rank>(attr)) {
        throw H5::AttributeIException("H5xx::attribute::as", "incompatible dataspace");
    }

    hsize_t dim[rank];
    ds.getSimpleExtentDims(dim);
    boost::array<size_t, rank> shape;
    std::copy(dim, dim + rank, shape.begin());
    boost::multi_array<value_type, rank> value(shape);
    attr.read(ctype<value_type>(), value.data());
    return value;
}

/*
 * create and write vector type attribute
 */
template <typename T>
typename boost::enable_if<is_vector<T>, attribute&>::type
attribute::operator=(T const& value)
{
    typedef typename T::value_type value_type;

    H5::Attribute attr;
    try {
        H5XX_NO_AUTO_PRINT(H5::AttributeIException);
        attr = m_node->openAttribute(m_name);
        if (!has_type<T>(attr) || elements(attr) != value.size()) {
            // recreate attribute with proper type
            m_node->removeAttr(m_name);
            throw H5::AttributeIException();
        }
    }
    catch (H5::AttributeIException const&) {
        hsize_t dim[1] = { value.size() };
        H5::DataSpace ds(1, dim);
        attr = m_node->createAttribute(m_name, ctype<value_type>(), ds);
    }
    attr.write(ctype<value_type>(), value.data());
    return *this;
}

/**
 * read vector type attribute
 *
 * read data of possibly higher rank into 1D std::vector
 */
template <typename T>
typename boost::enable_if<is_vector<T>, T>::type
attribute::as()
{
    typedef typename T::value_type value_type;

    H5::Attribute attr;
    try {
        H5XX_NO_AUTO_PRINT(H5::AttributeIException);
        attr = m_node->openAttribute(m_name);
    }
    catch (H5::AttributeIException const&) {
        throw;
    }

    H5::DataSpace ds(attr.getSpace());
    if (!has_type<T>(attr) || !ds.isSimple()) {
        throw H5::AttributeIException("H5xx::attribute::as", "incompatible dataspace");
    }
    size_t size = ds.getSimpleExtentNpoints();
    std::vector<value_type> value(size);
    attr.read(ctype<value_type>(), value.data());
    return value;
}

} // namespace H5xx

#endif /* ! HALMD_UTIL_H5XX_ATTRIBUTE_HPP */
