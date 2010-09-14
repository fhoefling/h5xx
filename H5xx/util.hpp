/* HDF5 C++ extensions
 *
 * Copyright © 2008-2009  Peter Colberg
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

#ifndef HALMD_UTIL_H5XX_UTIL_HPP
#define HALMD_UTIL_H5XX_UTIL_HPP

#define H5E_auto_t_vers 2
#include <H5Cpp.h>

#include <string>
#include <vector>

namespace H5xx
{

/**
 * returns absolute path of a HDF5 object within file
 *
 * For attributes the path of the parent object is returned.
 */
inline std::string path(H5::IdComponent const& id)
{
    ssize_t size; // excludes NULL terminator
    if (-1 == (size = H5Iget_name(id.getId(), NULL, 0))) {
        throw H5::IdComponentException("H5Iget_name", "failed to get length of name");
    }
    std::vector<char> name_(size + 1); // includes NULL terminator
    if (-1 == (H5Iget_name(id.getId(), name_.data(), name_.size()))) {
        throw H5::IdComponentException("H5Iget_name", "failed to get name");
    }
    return name_.data();
}

} // namespace H5xx

#endif /* ! HALMD_UTIL_H5XX_UTIL_HPP */
