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

#ifndef H5XX_PROPERTY_HPP
#define H5XX_PROPERTY_HPP

#include <h5xx/error.hpp>
#include <h5xx/hdf5_compat.hpp>

namespace h5xx {

/**
 * Create link creation property list and set create intermediate group property.
 */
inline H5::PropList create_intermediate_group_property()
{
    hid_t pl = H5Pcreate(H5P_LINK_CREATE);
    if (pl < 0) {
        throw error("failed to create link creation property list");
    }
    herr_t err = H5Pset_create_intermediate_group(pl, 1);
    if (err < 0) {
        throw error("failed to set group intermediate creation property");
    }
    return H5::PropList(pl);
}

} // namespace h5xx

#endif /* ! H5XX_PROPERTY_HPP */
