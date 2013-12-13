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

#ifndef H5XX_ATTRIBUTE_STD_VECTOR
#define H5XX_ATTRIBUTE_STD_VECTOR

#include <vector>

#include <h5xx/ctype.hpp>
#include <h5xx/error.hpp>
#include <h5xx/exception.hpp>
#include <h5xx/utility.hpp>
#include <h5xx/attribute/utility.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/mpl/and.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

/**
 * create and write std::vector attributes of fundamental type
 **/

namespace h5xx {

template <typename h5xxObject, typename T>
inline typename boost::enable_if< boost::mpl::and_<is_vector<T>, boost::is_fundamental<typename T::value_type> >, void>::type
write_attribute(h5xxObject const& object, std::string const& name, T const& value)
{
    typedef typename T::value_type value_type;
    bool err = false;
    char const* attr_name = name.c_str();
    hid_t attr_id;
    hsize_t dim[1] = {value.size()};
    if (exists_attribute(object, name))     //the attribute will be replaced if it already exists
    {
        delete_attribute(object, name);
    }
    hid_t space_id = H5Screate_simple(1, dim, dim);
    hid_t type_id = ctype<value_type>::hid();       //this ID can not be closed
    err |= (attr_id = H5Acreate(object.hid(), attr_name, type_id, space_id, H5P_DEFAULT, H5P_DEFAULT)) < 0;
    err |= H5Sclose(space_id) < 0;
    if (err) {
        throw error("creating attribute");
    }

    err |= H5Awrite(attr_id, type_id, &*value.begin()) < 0;

    err |= H5Aclose(attr_id) < 0;

    if (err) {
        throw error("writing attribute \"" + name + "\" with ID " + boost::lexical_cast<std::string>(attr_id));
    }
}

/**
 * read std::vector attributes of fundamental type
 **/

template <typename T, typename h5xxObject>
inline typename boost::enable_if<boost::mpl::and_<
    is_vector<T>, boost::is_fundamental<typename T::value_type>
>, T>::type
read_attribute(h5xxObject const& object, std::string const& name)
{
    typedef typename T::value_type value_type;
    char const* attr_name = name.c_str();
    hid_t attr_id;
    bool err = false;
    hsize_t dim[1];
    hsize_t maxdim[1];

    if(!exists_attribute(object, attr_name)) {
        throw error("Attribute does not exist");
    }

    // open object
    err |= (attr_id = H5Aopen(object.hid(), attr_name, H5P_DEFAULT)) < 0;
    if (err) {
        throw error("opening atrribute \"" + name + "\" with ID " + boost::lexical_cast<std::string>(attr_id));
    }
    if (!has_simple_space(attr_id)) {
        throw error("Attribute \"" + name + "\" of object \"" + get_name(object) + "\" has incompatible dataspace");
    }

    hid_t attr_space = H5Aget_space(attr_id);
    if ( !has_rank<1>(attr_space)) {
        throw error("Attribute has an invalid dataspace");
    }

    H5Sget_simple_extent_dims(attr_space, dim, maxdim);

    err |= H5Sclose(attr_space) < 0;

    std::vector<value_type> value(dim[0]);
    // read from opened object
    err |= (H5Aread(attr_id, ctype<value_type>::hid(), &*value.begin())) < 0;
    // close object
    err |= H5Aclose(attr_id) < 0;
    if (err) {
        throw error("reading atrribute \"" + name + "\" with ID " + boost::lexical_cast<std::string>(attr_id));
    }
    return value;
}


/**
 * create and write std::vector attributes of string type
 **/

template <typename h5xxObject, typename T>
inline typename boost::enable_if< boost::mpl::and_<is_vector<T>, boost::is_same<typename T::value_type, std::string> >, void>::type
write_attribute(h5xxObject const& object, std::string const& name, T const& value)
{
    bool err = false;
    char const* attr_name = name.c_str();
    hid_t attr_id;
    size_t size = value.size();
    hsize_t dim[1] = { size };
    if (exists_attribute(object, name))     //the attribute will be replaced if it already exists
    {
        delete_attribute(object, name);
    }

    // size of longest string
    size_t str_size = 0;
    for (size_t i = 0; i < size; ++i) {
        str_size = std::max(str_size, value[i].size());
    }
//     std::cout << (str_size);

    hid_t type_id = H5Tcopy(H5T_C_S1);
    err |= H5Tset_size(type_id, str_size) < 0;
    err |= H5Tset_strpad(type_id, H5T_STR_NULLTERM) < 0;
    hid_t space_id = H5Screate_simple(1, dim, dim);

    err |= (attr_id = H5Acreate(object.hid(), attr_name, type_id, space_id, H5P_DEFAULT, H5P_DEFAULT)) < 0;
    err |= H5Sclose(space_id) < 0;


    if (err) {
        throw error("creating attribute");
    }

    std::vector<char> buffer(size * str_size);
    for (size_t i = 0; i < size; ++i){
        value[i].copy(buffer.data() + i*str_size, str_size);
    }
//     for (size_t i = 0; i < size*str_size; ++i) {
//         std::cout << (buffer[i]);
//     }
    char const* p = &*buffer.begin();
    err |= H5Awrite(attr_id, type_id, p) < 0;
    err |= H5Tclose(type_id) < 0;
    err |= H5Aclose(attr_id) < 0;
    if (err) {
        throw error("writing attribute \"" + name + "\" with ID " + boost::lexical_cast<std::string>(attr_id));
    }
}

/**
 * read std::vector attributes of string type
 **/

template <typename T, typename h5xxObject>
inline typename boost::enable_if<boost::mpl::and_<
    is_vector<T>, boost::is_same<typename T::value_type, std::string>
>, T>::type
read_attribute(h5xxObject const& object, std::string const& name)
{
    char const* attr_name = name.c_str();
    hid_t attr_id;
    bool err = false;
    // open object

    if(!exists_attribute(object, attr_name)) {
        throw error("Attribute does not exist");
    }

    err |= (attr_id = H5Aopen(object.hid(), attr_name, H5P_DEFAULT)) < 0;
    if (err) {
        throw error("opening atrribute \"" + name + "\" with ID " + boost::lexical_cast<std::string>(attr_id));
    }

    if (!has_simple_space(attr_id) /*|| !has_rank<1>(attr_id))*/) {
        throw error("Attribute \"" + name + "\" of object \"" + get_name(object) + "\" has incompatible dataspace");
    }
    hid_t attr_space = H5Aget_space(attr_id);
    if ( !has_rank<1>(attr_space)) {
        throw error("Attribute has an invalid dataspace");
    }

    hid_t type_id;
    hid_t space_id;
    err |= (type_id = H5Aget_type(attr_id)) < 0;     //get copy of the attribute's type
    err |= (space_id = H5Aget_space(attr_id)) < 0;   //get copy of the attribute's space

    hid_t mem_type_id = H5Tcopy(H5T_C_S1);
    hsize_t str_size = H5Tget_size(type_id);
    hsize_t size = H5Sget_simple_extent_npoints(space_id);
    err |= H5Tset_size(mem_type_id, str_size) < 0;

    // read from opened object to buffer
    std::vector<char> buffer(str_size * size);
    err |= (H5Aread(attr_id, mem_type_id, &*buffer.begin())) < 0;
    if (err) {
        throw error("error while reading attribute \"" + name + "\"");
    }
    // close object
    err |= H5Aclose(attr_id) < 0;
    err |= H5Tclose(mem_type_id) < 0;
    err |= H5Tclose(type_id) < 0;
    err |= H5Sclose(space_id) < 0;
    //H5Tclose(memtype);
    if (err) {
        throw error("reading atrribute \"" + name + "\" with ID " + boost::lexical_cast<std::string>(attr_id));
    }
    T value;
    value.reserve(size);
    char const* s = buffer.data();
    for (size_t i = 0; i < size; ++i, s += str_size) {
        size_t len = strnlen(s, str_size); // strings of str_len size are not '\0'-terminated
        value.push_back(std::string(s, len)); // copy len bytes from buffer
    }
    return value;
}


} //namespace h5xx
#endif // ! H5XX_ATTRIBUTE_STD_VECTOR