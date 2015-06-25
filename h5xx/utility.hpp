/*
 * Copyright © 2008-2015 Felix Höfling
 * Copyright © 2014-2015 Klaus Reuter
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
#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>



#define SEPARATOR \
    std::cout << "--------------------------------------------------------------------------------" << std::endl << std::flush;

#define H5XX_WHERE \
    std::cout << __FILE__ << ":" << __LINE__ << std::endl << __PRETTY_FUNCTION__<< std::endl << std::flush;

#define H5XX_THROW(MSG) \
    { \
        std::string message; \
        message.append(__FILE__); \
        message.append(":"); \
        message.append(num2str(__LINE__)); \
        message.append(":"); \
        message.append(__FUNCTION__); \
        message.append("(): "); \
        message.append(MSG); \
        throw error(message); \
    }

/** some macros for printf-style debugging */
//#define H5XX_DEBUG
//
#ifdef H5XX_DEBUG
#define H5XX_CHKPT \
    SEPARATOR; \
    H5XX_WHERE; \
    SEPARATOR;
#define H5XX_PRINT(MSG) \
    SEPARATOR; \
    H5XX_WHERE; \
    std::cout << "MSG : " << /*std::string(MSG)*/ MSG << std::endl << std::flush; \
    SEPARATOR;
#else
#define H5XX_CHKPT
#define H5XX_PRINT(MSG)
#endif

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
 */
inline std::string get_name(hid_t hid)
{
    ssize_t size = H5Iget_name(hid, NULL, 0); // get size of string
    if (size < 0) {
        throw error("failed to get name of HDF5 object with ID " + boost::lexical_cast<std::string>(hid));
    }
    std::vector<char> buffer;
    buffer.resize(size + 1); // includes NULL terminator
    size = H5Iget_name(hid, &*buffer.begin(), buffer.size()); // get string data
    // restore the standard behaviour for HDF5 attributes
    if (H5Iget_type(hid) == H5I_ATTR)
    {
        // up to now, the buffer contains the name of the object to which the attribute is attached,
        // so let's extend it with the attribute's name to form a full path to the attribute
        ssize_t attr_size = H5Aget_name(hid, 0, NULL);
        if (attr_size < 0) {
            throw error("failed to get name of HDF5 attribute with ID " + boost::lexical_cast<std::string>(hid));
        }
        std::vector<char> attr_buffer;
        attr_buffer.resize(attr_size + 1); // includes NULL terminator
        attr_size = H5Aget_name(hid, attr_buffer.size(), &*attr_buffer.begin());
        if (buffer.back() == '\0')
            buffer.pop_back();
        if (buffer.back() != '/')
            buffer.push_back('/');
        std::copy(attr_buffer.begin(), attr_buffer.end(), buffer.end());
    }
    return &*buffer.begin(); // convert char* to std::string
}

template <typename h5xxObject>
inline std::string get_name(h5xxObject const& obj)
{
    return get_name(obj.hid());
}

///**
// * hard link HDF5 object into the given group with given name
// */
//inline void link(H5::H5Object const& object, H5::Group const& group, std::string const& name)
//{
//    if (0 > H5Lcreate_hard(object.getId(), ".", group.getId(), name.c_str(), H5P_DEFAULT, H5P_DEFAULT)) {
//        throw error("failed to link object");
//    }
//}

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


/** swaps two h5xx objects */
template <typename h5xxObject>
void swap(h5xxObject& left, h5xxObject& right)
{
    const hid_t tmp = left.hid_;
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


/**
 * Convert an object, typically a number, to a std::string.
 */
template <typename T>
inline std::string num2str(T const& num)
{
    std::ostringstream oss;
    oss << num;
    return oss.str();
}

/**
 * Convert a string to another object, typically to a number.
 */
template <typename T>
inline T str2num(std::string const& str)
{
    std::istringstream iss(str);
    T num;
    iss >> num;
    return num;
}

/**
 * Separate a string by a separator and return a vector of substrings.
 */
std::vector<std::string> chop(std::string const& str, std::string const& sep)
{
    std::vector<std::string> items;
    std::string::size_type begIdx, endIdx;
    begIdx = str.find_first_not_of(sep);
    while (begIdx != std::string::npos) {
        endIdx = str.find_first_of(sep, begIdx);
        if (endIdx == std::string::npos) {
            endIdx = str.length();
        }
        std::string buf; buf.clear();
        for (std::string::size_type i = begIdx; i<endIdx; ++i) {
            buf.push_back(str[i]);
        }
        items.push_back(buf);
        begIdx = str.find_first_not_of (sep, endIdx);
    }
    return items;
}

} // namespace h5xx




#endif /* ! H5XX_UTILITY_HPP */
