/* HDF5 C++ extensions
 *
 * Copyright © 2008-2010  Peter Colberg and Felix Höfling
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

#ifndef H5XX_GROUP_HPP
#define H5XX_GROUP_HPP

#include <h5xx/exception.hpp>
#include <h5xx/utility.hpp>

namespace h5xx
{

/**
 * open or create HDF5 group walking down the tree along path
 * path is given as range of iterators
 */
template <typename string_iterator>
inline H5::Group open_group(
    H5::CommonFG const& fg
  , string_iterator path_begin
  , string_iterator path_end
)
{
    H5::Group group;
    // open root if fg is a file
    if (typeid(fg) == typeid(H5::H5File)) {
        group = fg.openGroup("/");
    } else {
        group = dynamic_cast<H5::Group const&>(fg);
    }
    for (string_iterator it = path_begin; it != path_end; ++it) {
        // open or create group at each level
        try {
            H5XX_NO_AUTO_PRINT(H5::GroupIException);
            group = group.openGroup(*it);
        }
        catch (H5::GroupIException const&) {
            group = group.createGroup(*it);
        }
    }
    return group;
}

/**
 * open or create HDF5 group walking down the tree along path
 *
 * path_string is split at every occurrence of '/'
 */
inline H5::Group open_group(H5::CommonFG const& fg, std::string const& path_string)
{
    std::list<std::string> path = split_path(path_string);
    return open_group(fg, path.begin(), path.end());
}

} // namespace h5xx

#endif /* ! H5XX_GROUP_HPP */
