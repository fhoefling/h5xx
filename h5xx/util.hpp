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

#ifndef H5XX_UTIL_HPP
#define H5XX_UTIL_HPP

#include <boost/algorithm/string.hpp>
#include <string>
#include <list>

#include <h5xx/hdf5.hpp>

namespace H5
{

/**
 * returns absolute path of an HDF5 object within file
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

/**
 * split path_string on '/' and return list of group names,
 * empty names are suppressed
 */
inline std::list<std::string> split_path(std::string const& path_string)
{
    using namespace std;
    using namespace boost::algorithm;

    list<string> groups;
    split(groups, path_string, is_any_of("/"));  // equal('/')
    // drop empty strings (if path starts or ends with '/')
    for (list<string>::iterator s=groups.begin(); s != groups.end(); ) {
        if (*s == "") {
            groups.erase(s++);
        }
        else
            ++s;
    }

    return groups;
}

} // namespace H5

#endif /* ! H5XX_UTIL_HPP */
