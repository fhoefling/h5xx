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

#ifndef H5XX_ATTRIBUTE_BOOST_ARRAY
#define H5XX_ATTRIBUTE_BOOST_ARRAY

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
 * create and write fixed-size fundamental array type attribute for h5xx objects
 */
template <typename T, typename h5xxObject>
inline typename boost::enable_if<boost::mpl::and_<is_array<T>, boost::is_fundamental<typename T::value_type> >, void>::type
write_attribute(h5xxObject const& object, std::string const& name, T const& value)
{
    typedef typename T::value_type value_type;
    enum { size = T::static_size };
    bool err = false;
    char const* attr_name = name.c_str();
    hid_t attr_id;

    if (exists_attribute(object, name))     //the attribute will be replaced if it already exists
    {
        delete_attribute(object, name);
    }
    hsize_t dim[1] = {size};
    hid_t space_id = H5Screate_simple(1, dim, dim);
    err |= (attr_id = H5Acreate(object.hid(), attr_name, ctype<value_type>::hid(), space_id, H5P_DEFAULT, H5P_DEFAULT)) < 0;
    err |= H5Sclose(space_id) < 0;
    // write data
    err |= H5Awrite(attr_id, ctype<value_type>::hid(), &*value.begin()) < 0;
    err |= H5Aclose(attr_id) < 0;
    if (err) {
        throw error("writing attribute \"" + name + "\" with ID " + boost::lexical_cast<std::string>(attr_id));
    }
}

/**
 * read fixed-size fundamental array type attributes from h5xx objects
 */
template <typename T, typename h5xxObject>
inline typename boost::enable_if<boost::mpl::and_<is_array<T>, boost::is_fundamental<typename T::value_type> >, T>::type
read_attribute(h5xxObject const& object, std::string const& name)
{

    typedef typename T::value_type value_type;
    enum { size = T::static_size };
    char const* attr_name = name.c_str();
    hid_t attr_id;
    bool err = false;

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
    if ( !has_extent<T>(attr_space)  ) {
        throw error("Dimension of given type and attribute are not matching");

    }
    err |= H5Sclose(attr_space) < 0;

    T value;
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
 * create and write std::string arrays
 */
template <typename T, typename h5xxObject, typename StringPolicy>
inline typename boost::enable_if<boost::mpl::and_<is_array<T>, boost::is_same<typename T::value_type, std::string> >, void>::type
write_attribute(h5xxObject const& object, std::string const& name, T const& value, StringPolicy policy = StringPolicy())
{
    enum { size = T::static_size };
    hsize_t dim[1] = { size };
    bool err = false;
    char const* attr_name = name.c_str();
    hid_t attr_id;

    // remove attribute if it exists
    if (exists_attribute(object, name)) {
        delete_attribute(object, name);
    }
    size_t str_size = 0;

    for (hsize_t i = 0; i < size; ++i) {
        str_size = std::max(str_size, value[i].size());  // include terminating NULL character
    }

    hid_t space_id = H5Screate_simple(1, dim, dim);
    hid_t type_id = policy.make_type(str_size);
    assert(space_id >= 0);
    assert(type_id >= 0);
    err |= (attr_id = H5Acreate(object.hid(), attr_name, type_id, space_id, H5P_DEFAULT, H5P_DEFAULT)) < 0;
    if (err) {
        throw error("creating attribute \"" + name + "\" for object \"" + get_name(object) + "\"");
    }
    H5Sclose(space_id);


    if (StringPolicy::is_variable_length) {
        std::vector<const char*> buffer(size);
        for (size_t i = 0; i < size; i++) {
            buffer[i] = value[i].c_str();
        }
        err |= H5Awrite(attr_id, type_id, &*buffer.begin()) < 0;
    }
    else {
        std::vector<char> buffer(size * str_size);
        for (size_t i = 0; i < size; ++i) {
            value[i].copy(buffer.data() + i*str_size, str_size);
        }
        err |= H5Awrite(attr_id, type_id, &*buffer.begin()) < 0;
    }
    H5Tclose(type_id);
    H5Aclose(attr_id);

    if (err) {
        throw error("writing attribute \"" + name + "\" with ID " + boost::lexical_cast<std::string>(attr_id));
    }


}

template <typename T, typename h5xxObject>
inline typename boost::enable_if<boost::mpl::and_<is_array<T>, boost::is_same<typename T::value_type, std::string> >, void>::type
write_attribute(h5xxObject const& object, std::string const& name, T const& value)
{
    write_attribute(object, name, value, policy::string::null_terminated());
}

/**
 * read string array
 */
template <typename T, typename h5xxObject>
inline typename boost::enable_if<boost::mpl::and_<is_array<T>, boost::is_same<typename T::value_type, std::string> >, T>::type
read_attribute(h5xxObject const& object, std::string const& name)
{
    enum { size = T::static_size };
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
    if ( !has_extent<T>(attr_space)  ) {
        throw error("Dimension of given type and attribute are not matching");
    }

    hid_t type_id;
    err |= (type_id= H5Aget_type(attr_id)) < 0;     //get copy of the attribute's type

    htri_t is_varlen_str;
    err |= (is_varlen_str = H5Tis_variable_str(type_id)) < 0;
    if (err) {
        throw error("attribute \"" + name + "\" is not a valid string type");
    }

    T value;
    if (!is_varlen_str){
        hid_t mem_type_id = H5Tcopy(H5T_C_S1);
        hsize_t str_size = H5Tget_size(type_id) + 1;    //one extra char for nullpad and spacepad
        err |= H5Tset_size(mem_type_id, str_size) < 0;

        // read from opened object
        std::vector<char> buffer(str_size * size);
        err |= (H5Aread(attr_id, mem_type_id, &*buffer.begin())) < 0;
        if (err) {
            throw error("error while reading attribute \"" + name + "\"");
        }
        err |= H5Tclose(mem_type_id) < 0;

        char const* s = buffer.data();
        for (unsigned int i = 0; i < size; ++i, s += str_size) {
            size_t len = strnlen(s, str_size);
            value[i] = std::string(s, len);
        }
    }else {
        hid_t mem_type_id = H5Tget_native_type(type_id, H5T_DIR_ASCEND);
        std::vector<const char*> buffer(size);
        err |= (H5Aread(attr_id, mem_type_id, &*buffer.begin())) < 0;
        for (int i = 0; i < size; i++){
            value[i] = buffer[i];
        }
        err |= H5Tclose(mem_type_id) < 0;
    }

    err |= H5Tclose(type_id) < 0;
    err |= H5Aclose(attr_id) < 0;

    if (err) {
            throw error("reading atrribute \"" + name + "\" with ID " + boost::lexical_cast<std::string>(attr_id));
    }

    return value;
}


/**
* create and write c string array types from h5xx objects
*/
template <typename T, typename h5xxObject, typename StringPolicy>
inline typename boost::enable_if<boost::mpl::and_<is_array<T>, boost::is_same<typename T::value_type, char const*> >, void>::type
write_attribute(h5xxObject const& object, std::string const& name, T const& value, StringPolicy policy = StringPolicy())
{
    enum { size = T::static_size };
    hsize_t dim[1] = { size };
    bool err = false;
    char const* attr_name = name.c_str();
    hid_t attr_id;

    size_t str_size = 0;
    for (hsize_t i = 0; i < size; ++i) {
        str_size = std::max(str_size, strlen(value[i]) + 1);  // include terminating NULL character
    }

    // remove attribute if it exists
    if (exists_attribute(object, name)) {
        delete_attribute(object, name);
    }

    hid_t space_id = H5Screate_simple(1, dim, dim);
    hid_t type_id = policy.make_type(str_size);
    assert(space_id >= 0);
    assert(type_id >= 0);
    err |= (attr_id = H5Acreate(object.hid(), attr_name, type_id, space_id, H5P_DEFAULT, H5P_DEFAULT)) < 0;
    if (err) {
        throw error("creating attribute \"" + name + "\" for object \"" + get_name(object) + "\"");
    }
    err |= H5Sclose(space_id) < 0;

    if (StringPolicy::is_variable_length) {
        err |= H5Awrite(attr_id, type_id, &*value.begin()) < 0;
    }
    else {
        std::vector<char> data(str_size * size);
        for (size_t i = 0; i < size; ++i) {
            strncpy(&*data.begin() + i * str_size, value[i], str_size);
        }
        err |= H5Awrite(attr_id, type_id, &*data.begin()) < 0;
    }

    err |= H5Tclose(type_id) < 0;
    err |= H5Aclose(attr_id) < 0;

    if (err) {
        throw error("writing attribute \"" + name + "\" with ID " + boost::lexical_cast<std::string>(attr_id));
    }
}

template <typename T, typename h5xxObject>
inline typename boost::enable_if<boost::mpl::and_<is_array<T>, boost::is_same<typename T::value_type, char const*> >, void>::type
write_attribute(h5xxObject const& object, std::string const& name, T const& value)
{
    write_attribute(object, name, value, policy::string::null_terminated());
}

} //namespace h5xx
#endif // ! H5XX_ATTRIBUTE_BOOST_ARRAY
