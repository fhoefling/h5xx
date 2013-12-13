/*
 * Copyright © 2014 Felix Höfling
 * Copyright © 2014  Manuel Dibak
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

#ifndef H5XX_ATTRIBUTE_SCALAR
#define H5XX_ATTRIBUTE_SCALAR

#include <h5xx/ctype.hpp>
#include <h5xx/error.hpp>
#include <h5xx/exception.hpp>
#include <h5xx/utility.hpp>
#include <h5xx/attribute/utility.hpp>
#include <h5xx/policy/string.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/mpl/and.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

namespace h5xx {

 /**
*create and write fundamental type attribute on h5xx objects
*/
template <typename h5xxObject, typename T>
inline typename boost::enable_if<boost::is_fundamental<T>, void>::type
write_attribute(h5xxObject const& object, std::string const& name, T const& value)
{
    bool err = false;
    char const* attr_name = name.c_str();

    // delete attribute if it existed before
    if (exists_attribute(object, attr_name)) {
        delete_attribute(object, name);
    }

    // (re)create attribute
    hid_t space_id = H5Screate(H5S_SCALAR);
    hid_t attr_id = H5Acreate(object.hid(), attr_name, ctype<T>::hid(), space_id, H5P_DEFAULT, H5P_DEFAULT);
    err |= H5Sclose(space_id) < 0;

    err |= attr_id < 0;
    // write data
    err |= H5Awrite(attr_id, ctype<T>::hid(), &value) < 0;
    err |= H5Aclose(attr_id) < 0;
    if (err) {
        throw error("writing attribute \"" + name + "\" with ID " + boost::lexical_cast<std::string>(attr_id));
    }
}

/**
 * read fundamental type attribute of a given h5xx::object
 */
template <typename T, typename h5xxObject>
inline typename boost::enable_if<boost::is_fundamental<T>, T>::type
read_attribute(h5xxObject const& object, std::string const& name)
{
    char const* attr_name = name.c_str();
    hid_t attr_id;
    bool err = false;

    if(!exists_attribute(object, attr_name)) {
        throw error("Attribute does not exist");
    }
    // open object
    err |= (attr_id = H5Aopen(object.hid(), attr_name, H5P_DEFAULT)) < 0;
    if (!has_scalar_space(attr_id)) {
        throw error("Attribute \"" + name + "\" of object \"" + get_name(object) + "\" has incompatible dataspace");
    }
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
 * create and write std::string attribute for h5xx object
 */
template <typename T, typename h5xxObject, typename StringPolicy> // only in C++11: StringPolicy = policy::string::null_terminated
inline typename boost::enable_if<boost::is_same<T, std::string>, void>::type
write_attribute(h5xxObject const& object, std::string const& name, T const& value, StringPolicy policy = StringPolicy())
{
    char const* attr_name = name.c_str();
    bool err = false;
    hid_t space_id = H5Screate(H5S_SCALAR);
    hid_t type_id = policy.make_type(value.size());
    hid_t attr_id;

    // open or create attribute
    if (exists_attribute(object, name)) {
        delete_attribute(object, name);
    }

    err |= (attr_id = H5Acreate(object.hid(), attr_name, type_id, space_id, H5P_DEFAULT, H5P_DEFAULT)) < 0;
    err |= H5Sclose(space_id) < 0;

    if (err) {
        throw error("creating attribute");
    }
    // write data
    if (StringPolicy::is_variable_length) {
        char const* p = value.c_str();
        err |= H5Awrite(attr_id, type_id, &p) < 0;
    }
    else {
        err |= H5Awrite(attr_id, type_id, &*value.data()) < 0;
    }
    err |= H5Aclose(attr_id) < 0;
    err |= H5Tclose(type_id) < 0;
    if (err) {
        throw error("writing attribute \"" + name + "\" with ID " + boost::lexical_cast<std::string>(attr_id));
    }
}

template <typename T, typename h5xxObject>
inline typename boost::enable_if<boost::is_same<T, std::string>, void>::type
write_attribute(h5xxObject const& object, std::string const& name, T const& value)
{
    write_attribute(object, name, value, policy::string::null_terminated());
}

/**
 * create and write C string attribute for h5xx objects
 */
template <typename T, typename h5xxObject, typename StringPolicy>
inline typename boost::enable_if<boost::is_same<T, char const*>, void>::type
write_attribute(h5xxObject const& object, std::string const& name, T value, StringPolicy policy = StringPolicy())
{
    char const* attr_name = name.c_str();
    bool err = false;
    htri_t attr_id;

    // delete attribute if it already exists
    if (exists_attribute(object, attr_name) > 0) {
        delete_attribute(object, name);
    }

    hid_t space_id = H5Screate(H5S_SCALAR);
    hid_t type_id = policy.make_type(strlen(value));

    err |= (attr_id = H5Acreate(object.hid(), attr_name, type_id, space_id, H5P_DEFAULT, H5P_DEFAULT)) < 0 ;
    err |=  H5Sclose(space_id) < 0;

    // write data

    if (StringPolicy::is_variable_length) {
        err |= H5Awrite(attr_id, type_id, &value) < 0;
    }
    else {
        err |= H5Awrite(attr_id, type_id, &*value) < 0;
    };

    err |= H5Aclose(attr_id) < 0;
    err |= H5Tclose(type_id) < 0;
    if (err) {
        throw error("writing attribute \"" + name + "\" with ID " + boost::lexical_cast<std::string>(attr_id));
    }
}

template <typename T, typename h5xxObject>
inline typename boost::enable_if<boost::is_same<T, char const*>, void>::type
write_attribute(h5xxObject const& object, std::string const& name, T value)
{
    write_attribute(object, name, value, policy::string::null_terminated());
}

/**
 * read std::string attribute for h5xx objects
 */
template <typename T, typename h5xxObject>
inline typename boost::enable_if<boost::is_same<T, std::string>, T>::type
read_attribute(h5xxObject const& object, std::string const& name)
{
    char const* attr_name = name.c_str();
    hid_t attr_id;
    bool err = false;

    if(!exists_attribute(object, attr_name)) {
        throw error("Attribute does not exist");
    }

    // open object
    err |= (attr_id = H5Aopen(object.hid(), attr_name, H5P_DEFAULT)) < 0;
    if (err) {
        throw error("Can not open attribute \""+name+"\"");
    }
    if (!has_scalar_space(attr_id)) {
        throw error("Attribute \"" + name + "\" of object \"" + get_name(object) + "\" has incompatible dataspace");
    }

    hid_t type_id;
    err |= (type_id= H5Aget_type(attr_id)) < 0;     //get copy of the attribute's type

    // check if string is of variable or fixed size type
    htri_t is_varlen_str;
    err |= (is_varlen_str =  H5Tis_variable_str(type_id)) < 0;
    if (err) {
        throw error("attribute \"" + name + "\" is not a valid string type");
    }

    std::string value;
    if (!is_varlen_str) {
        // read fixed-size string, allocate space in advance and let the HDF5
        // library take care about NULLTERM and NULLPAD strings
        hsize_t size = H5Tget_size(type_id);
        hid_t mem_type_id = H5Tcopy(H5T_C_S1);
        err |= H5Tset_size(mem_type_id, size + 1) < 0;  //one extra character for zero- and space-padded strings
        if (err) {
            throw error("Setting size of mem_type_id");
        }
        value.resize(size, std::string::value_type());
        err |= H5Aread(attr_id, mem_type_id, &*value.begin());
        err |= H5Tclose(mem_type_id);

    }  else {
        // read variable-length string, memory will be allocated by HDF5 C
        // library and must be freed by us
        hid_t mem_type_id = H5Tget_native_type(type_id, H5T_DIR_ASCEND);
        char *c_str;
        err |= (H5Aread(attr_id, mem_type_id, &c_str) < 0);
        if (!err) {
            value = c_str;  // copy '\0'-terminated string
            free(c_str);
        }
    }

    err |= H5Tclose(type_id);
    err |= H5Aclose(attr_id);
    if (err) {
        throw error("reading atrribute \"" + name + "\" with ID " + boost::lexical_cast<std::string>(attr_id));
    }
    return value;
}

} // namespace h5xx
#endif // ! H5XX_ATTRIBUTE_SCALAR