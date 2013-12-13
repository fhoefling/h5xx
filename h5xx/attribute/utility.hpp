/* HDF5 C++ extensions
 *
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

#ifndef H5XX_ATTRIBUTE_UTILITY_HPP
#define H5XX_ATTRIBUTE_UTILITY_HPP

#include <h5xx/ctype.hpp>
#include <h5xx/error.hpp>
#include <h5xx/exception.hpp>
#include <h5xx/utility.hpp>

#include <boost/lexical_cast.hpp>

namespace h5xx {


/**
 * Check whether an attribute exists for an object
 * given by either the object hid or as a h5xx Object
 **/

template <typename h5xxObject>
inline bool exists_attribute(h5xxObject const& object, std::string const& name)
{
    hid_t obj_id = object.hid();
    htri_t tri = H5Aexists(obj_id, name.c_str());
    if (tri < 0) {
        throw error("testing attribute \"" + name + "\" at object ID " + boost::lexical_cast<std::string>(obj_id));
    }
    return (tri > 0);
}


/**
 * Delete attribute from group, dataset, or datatype
 * given as by either its object hid or as a h5xx object
 *
 * warning: no other attribute of the object must be opened
 */

template <typename h5xxObject>
inline void delete_attribute(h5xxObject const& object, std::string const& name)
{
    hid_t obj_id = object.hid();
    if (exists_attribute(object, name)) {
        if (H5Adelete(obj_id, name.c_str()) < 0) {
            throw error("deleting attribute \"" + name + "\" for object \"" + get_name(obj_id) + "\"");
        }
    }
    else {
        throw error("to be deleted attribute \"" + name + "\"  does not exist for object \"" + get_name(obj_id) + "\"");
    }
}
} //namespace h5xx

#endif /* ! H5XX_ATTRIBUTE_UTILITY_HPP */