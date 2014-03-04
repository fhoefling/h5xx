/*
 * Copyright © 2014 Manuel Dibak
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

#ifndef H5XX_ATTRIBUTE_UTILITY_HPP
#define H5XX_ATTRIBUTE_UTILITY_HPP

#include <h5xx/error.hpp>
#include <h5xx/utility.hpp>

namespace h5xx {

/**
 * Check whether an attribute of the given name is attached to the h5xx object.
 */
template <typename h5xxObject>
inline bool exists_attribute(h5xxObject const& obj, std::string const& name)
{
    htri_t tri = H5Aexists(obj.hid(), name.c_str());
    if (tri < 0) {
        throw error("testing attribute \"" + name + "\" at HDF5 object \"" + get_name(obj) + "\"");
    }
    return (tri > 0);
}

/**
 * Delete the attribute of the given name from the h5xx object. Non-existence
 * of the attribute is not an error.
 *
 * Warning: no other attribute of the object must be opened.
 */
template <typename h5xxObject>
inline void delete_attribute(h5xxObject const& obj, std::string const& name)
{
    if (exists_attribute(obj, name)) {
        if (H5Adelete(obj.hid(), name.c_str()) < 0) {
            throw error("deleting attribute \"" + name + "\" from HDF5 object \"" + get_name(obj) + "\"");
        }
    }
}

namespace detail {

/**
 * Returns HDF5 handle for data space of given attribute. Must be closed by
 * calling H5Sclose() after use. FIXME add class h5xx::dataspace
 */
inline hid_t get_dataspace(attribute const& attr) {
    hid_t space_id;
    if ((space_id = H5Aget_space(attr.hid())) < 0) {
        throw error ("can not get dataspace of attribute \"" + get_name(attr) + "\"");
    }
    return space_id;
}

/**
 * Returns true if the data space of the attribute is of scalar type.
 */
inline bool has_scalar_dataspace(attribute const& attr)
{
    if (!attr.valid()) {
        return false;
    }

    hid_t space_id = get_dataspace(attr);
    bool ret = H5Sget_simple_extent_type(space_id) == H5S_SCALAR;
    H5Sclose(space_id);
    return ret;
}

/**
 * Returns true if the data space of the attribute is of simple type.
 */
inline bool has_simple_dataspace(attribute const& attr)
{
    if (!attr.valid()) {
        return false;
    }

    hid_t space_id = get_dataspace(attr);
    bool ret = H5Sget_simple_extent_type(space_id) == H5S_SIMPLE;
    H5Sclose(space_id);
    return ret;
}

} // namespace detail

/**
 * FIXME remove this function
 * h5xx implementation to check whether a data space is scalar
 */
inline bool has_scalar_space(hid_t attr_id)
{
    hid_t space_id;
    if ((space_id = H5Aget_space(attr_id)) < 0) {
        throw error ("can not get dataspace of attribute with id " + boost::lexical_cast<std::string>(attr_id));
    }
    return H5Sget_simple_extent_type(space_id) == H5S_SCALAR;
}


/**
 * FIXME remove this function
 * h5xx implementation to check whether a data space is simple
 */
inline bool has_simple_space(hid_t attr_id)
{
    hid_t space_id;
    if ((space_id = H5Aget_space(attr_id)) < 0) {
        throw error ("can not get dataspace of attribute with id " + boost::lexical_cast<std::string>(attr_id));
    }
    return H5Sget_simple_extent_type(space_id) == H5S_SIMPLE;
}

} // namespace h5xx

#endif /* ! H5XX_ATTRIBUTE_UTILITY_HPP */
