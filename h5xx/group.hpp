/*
 * Copyright © 2008-2010  Peter Colberg and Felix Höfling
 * All rights reserved.
 *
 * This file is part of h5xx — a C++ wrapper for the HDF5 library.
 *
 * This software may be modified and distributed under the terms of the
 * 3-clause BSD license.  See accompanying file LICENSE for details.
 */

#ifndef H5XX_GROUP_HPP
#define H5XX_GROUP_HPP

#include <h5xx/error.hpp>
#include <h5xx/property.hpp>

namespace h5xx {

/**
 * determine whether group exists in file or group
 */
inline bool exists_group(H5::CommonFG const& fg, std::string const& name)
{
    H5::IdComponent const& loc(dynamic_cast<H5::IdComponent const&>(fg));
    hid_t hid;
    H5E_BEGIN_TRY {
        hid = H5Gopen(loc.getId(), name.c_str(), H5P_DEFAULT);
        if (hid > 0) {
            H5Gclose(hid);
        }
    } H5E_END_TRY
    return (hid > 0);
}

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

} // namespace h5xx

#endif /* ! H5XX_GROUP_HPP */
