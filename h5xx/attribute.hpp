/* HDF5 C++ extensions
 *
 * Copyright © 2008-2013  Felix Höfling
 * Copyright © 2013       Manuel Dibak
 * Copyright © 2008-2010  Peter Colberg
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

#ifndef H5XX_ATTRIBUTE_HPP
#define H5XX_ATTRIBUTE_HPP

#include <h5xx/ctype.hpp>
#include <h5xx/error.hpp>
#include <h5xx/exception.hpp>
#include <h5xx/utility.hpp>

#include <boost/any.hpp>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/mpl/and.hpp>
#include <boost/multi_array.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

#include <vector>

namespace h5xx {

/**
 * determine whether attribute exists in group, dataset, or datatype
 *
 * @param object Object ...
 * @param name attribute name
 * @return: true if attribute exists
 */
inline bool exists_attribute(H5::H5Object const& object, std::string const& name)
{
    hid_t obj_id = object.getId();
    htri_t tri = H5Aexists(obj_id, name.c_str());
    if (tri < 0) {
        throw error("testing attribute \"" + name + "\" at object ID " + boost::lexical_cast<std::string>(obj_id));
    }
    return (tri > 0);
}

/**
 * delete attribute from group, dataset, or datatype
 *
 * warning: no other attribute of the object must be opened
 */
inline void delete_attribute(H5::H5Object const& object, std::string const& name)
{
    hid_t obj_id = object.getId();
    if (H5Adelete(obj_id, name.c_str()) < 0) {
        throw error("deleting attribute \"" + name + "\" at object ID " + boost::lexical_cast<std::string>(obj_id));
    }
} // FIXME add test


/*
 * create and write fundamental type attribute
 */
template <typename T>
inline typename boost::enable_if<boost::is_fundamental<T>, H5::Attribute>::type
write_attribute(H5::H5Object const& object, std::string const& name, T const& value)
{
    hid_t loc_id = object.getId();
    char const* attr_name = name.c_str();
    hid_t attr_id;
    bool err = false;

    // open or create attribute
    bool do_create = true;
    if (exists_attribute(object, name)) {
        err |= (attr_id = H5Aopen(loc_id, attr_name, H5P_DEFAULT)) < 0;
        H5::Attribute attr(attr_id);
        attr.incRefCount();
        if (!has_type<T>(attr) || !has_scalar_space(attr)) {
            // recreate attribute with proper type
            err |= H5Aclose(attr_id) < 0;
            attr.decRefCount();
            delete_attribute(object, name);
        }
        else {
            // reuse existing attribute
            do_create = false;
        }
    }
    if (do_create) {
        hid_t space_id = H5Screate(H5S_SCALAR);
        attr_id = H5Acreate(loc_id, attr_name, ctype<T>::hid(), space_id, H5P_DEFAULT, H5P_DEFAULT);
        H5Sclose(space_id);
    }
    err |= attr_id < 0;

    // write data
    err |= H5Awrite(attr_id, ctype<T>::hid(), &value) < 0;
    err |= H5Aclose(attr_id) < 0;
    if (err) {
        throw error("writing attribute \"" + name + "\" with ID " + boost::lexical_cast<std::string>(attr_id));
    }
    return attr_id;
}

/**
 * read fundamental type attribute
 */
template <typename T>
inline typename boost::enable_if<boost::is_fundamental<T>, T>::type
read_attribute(H5::H5Object const& object, std::string const& name)
{
    hid_t loc_id = object.getId();
    char const* attr_name = name.c_str();
    hid_t attr_id;
    H5::Attribute attr;
    bool err = false;
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
    // open object
    err |= (attr_id = H5Aopen(loc_id, attr_name, H5P_DEFAULT)) < 0;
    T value;
    // read from opened object
    err |= (H5Aread(attr_id,ctype<T>::hid(), &value)) < 0;
    // close object
    err |= H5Aclose(attr_id) < 0;
    if (err) {
        throw error("reading atrribute \"" + name + "\" with ID " + boost::lexical_cast<std::string>(attr_id));
    }
    return value;
}

/**
 * create and write string attribute
 */
template <typename T>
inline typename boost::enable_if<boost::is_same<T, std::string>, void>::type
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
inline typename boost::enable_if<boost::is_same<T, std::string>, T>::type
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
    H5::DataType tid = attr.getDataType();
    std::string value;
    if (!tid.isVariableStr()) {
        // read fixed-size string, allocate space in advance and let the HDF5
        // library take care about NULLTERM and NULLPAD strings
        value.resize(tid.getSize(), std::string::value_type());
        attr.read(tid, &*value.begin());
    }
    else {
        // read variable-length string, memory will be allocated by HDF5 C
        // library and must be freed by us
        char *c_str;
        if (H5Aread(attr.getId(), tid.getId(), &c_str) < 0) {
            throw H5::AttributeIException("Attribute::read", "H5Aread failed");
        }
        value = c_str;  // copy '\0'-terminated string
        free(c_str);
    }
    return value;
}

/**
 * create and write C string attribute
 */
template <typename T>
inline typename boost::enable_if<boost::is_same<T, char const*>, void>::type
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
inline typename boost::enable_if<boost::mpl::and_<is_array<T>, boost::is_fundamental<typename T::value_type> >, void>::type
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
        attr = object.createAttribute(name, ctype<value_type>::hid_copy(), ds);
    }
    attr.write(ctype<value_type>::hid_copy(), &*value.begin());
}

/*
 * create and write fixed-size C string array type attribute
 */
template <typename T>
inline typename boost::enable_if<boost::mpl::and_<is_array<T>, boost::is_same<typename T::value_type, char const*> >, void>::type
write_attribute(H5::H5Object const& object, std::string const& name, T const& value)
{
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
        strncpy(&*data.begin() + i * max_len, value[i], max_len);
    }
    attr.write(tid, &*data.begin());
}

/**
 * read fixed-size array type attribute
 */
template <typename T>
inline typename boost::enable_if<boost::mpl::and_<is_array<T>, boost::is_fundamental<typename T::value_type> >, T>::type
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
    attr.read(ctype<value_type>::hid_copy(), &*value.begin());
    return value;
}

/*
 * create and write multi-dimensional array type attribute
 */
template <typename T>
inline typename boost::enable_if<is_multi_array<T>, void>::type
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
        attr = object.createAttribute(name, ctype<value_type>::hid_copy(), ds);
    }
    attr.write(ctype<value_type>::hid_copy(), value.origin());
}

/**
 * read multi-dimensional array type attribute
 */
template <typename T>
inline typename boost::enable_if<is_multi_array<T>, T>::type
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
    attr.read(ctype<value_type>::hid_copy(), value.origin());
    return value;
}

/*
 * create and write vector type attribute
 */
template <typename T>
inline typename boost::enable_if<is_vector<T>, void>::type
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
        attr = object.createAttribute(name, ctype<value_type>::hid_copy(), ds);
    }
    attr.write(ctype<value_type>::hid_copy(), &*value.begin());
}

/**
 * read vector type attribute
 *
 * read data of possibly higher rank into 1D std::vector
 */
template <typename T>
inline typename boost::enable_if<is_vector<T>, T>::type
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
    attr.read(ctype<value_type>::hid_copy(), &*value.begin());
    return value;
}

/**
 * returns attribute value as boost::any if exists, or empty boost::any otherwise
 */
template <typename T>
inline boost::any read_attribute_if_exists(H5::H5Object const& object, std::string const& name)
{
    if (exists_attribute(object, name)) {
        return read_attribute<T>(object, name);
    }
    return boost::any();
}

} // namespace h5xx

#endif /* ! H5XX_ATTRIBUTE_HPP */
