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
 *
 * @param object    one of h5xx::group, h5xx::dataset, or h5xx::datatype
 * @param name      attribute name
 * @returns         true if attribute exists
 */
template <typename h5xxObject>
inline bool exists_attribute(h5xxObject const& object, std::string const& name)
{
    // return false also if existence can not be queried
    return H5Aexists(object.hid(), name.c_str()) > 0;
}

/**
 * Delete the attribute of the given name from the h5xx object. Non-existence
 * of the attribute is not an error.
 *
 * Warning: no other attribute of the object must be opened.
 *
 * @param object    one of h5xx::group, h5xx::dataset, or h5xx::datatype
 * @param name      attribute name
 */
template <typename h5xxObject>
inline void delete_attribute(h5xxObject const& object, std::string const& name)
{
    if (exists_attribute(object, name)) {
        if (H5Adelete(object.hid(), name.c_str()) < 0) {
            throw error("deleting attribute \"" + name + "\" from HDF5 object \"" + get_name(object) + "\"");
        }
    }
}

} // namespace h5xx

#endif /* ! H5XX_ATTRIBUTE_UTILITY_HPP */
