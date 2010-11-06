/*
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

#include <boost/algorithm/string/join.hpp> // FIXME deprecated

#include <h5xx/error.hpp>
#include <h5xx/property.hpp>

namespace h5xx
{

/**
 * open or create HDF5 group
 *
 * This function creates missing intermediate groups.
 */
inline H5::Group open_group(H5::CommonFG const& fg, std::string const& path)
{
    H5::IdComponent const& loc(dynamic_cast<H5::IdComponent const&>(fg));
    hid_t group_id;
    H5E_BEGIN_TRY {
        group_id = H5Gopen(loc.getId(), path.c_str(), H5P_DEFAULT);
    } H5E_END_TRY
    if (group_id < 0) {
        H5::PropList pl = create_intermediate_group_property();
        group_id = H5Gcreate(loc.getId(), path.c_str(), pl.getId(), H5P_DEFAULT, H5P_DEFAULT);
    }
    if (group_id < 0) {
        throw error("failed to create group \"" + path + "\"");
    }
    return H5::Group(group_id);
}

// FIXME deprecated
template <typename string_iterator>
H5::Group open_group(
    H5::CommonFG const& fg
  , string_iterator path_begin
  , string_iterator path_end
)
{
    std::string path = boost::algorithm::join(std::vector<std::string>(path_begin, path_end), "/");
    if (path.empty()) {
        return open_group(fg, "/");
    }
    return open_group(fg, path);
}


} // namespace h5xx

#endif /* ! H5XX_GROUP_HPP */
