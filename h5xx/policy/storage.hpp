/*
 * Copyright © 2014 Felix Höfling
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

#ifndef H5XX_POLICY_STORAGE_HPP
#define H5XX_POLICY_STORAGE_HPP

#include <boost/array.hpp>
#include <vector>

#include <h5xx/error.hpp>
#include <h5xx/h5xx.hpp>

namespace h5xx {
namespace policy {
namespace storage {

/**
 * policy class to specify a contiguous dataset layout
 */
struct contiguous : public h5xx::policy::dataset_creation_property
{
    contiguous() {}

    /** set compact storage for given property list */
    void set(hid_t plist) const
    {
        if (H5Pset_layout(plist, H5D_CONTIGUOUS) < 0) {
            throw error("setting contiguous dataset layout failed");
        }
    }
};

/**
 * policy class to specify a compact dataset layout
 */
struct compact : public h5xx::policy::dataset_creation_property
{
    compact() {}

    /** set compact storage for given property list */
    void set(hid_t plist) const
    {
        if (H5Pset_layout(plist, H5D_COMPACT) < 0) {
            throw error("setting compact dataset layout failed");
        }
    }
};

class chunked : public h5xx::policy::dataset_creation_property
{
public:
    /**
     * specify the size, in dataset elements, of a chunk in each dimension
     */
    chunked(std::vector <hsize_t> const& dims)
      : dims_(dims) {}

    /**
     * specify the size, in dataset elements, of a chunk in each dimension
     * and the filter pipeline
     */

    /** set chunked storage layout for given property list */
    void set(hid_t plist) const
    {
        bool err = false;
        err |= H5Pset_layout(plist, H5D_CHUNKED) < 0;
        err |= H5Pset_chunk(plist, dims_.size(), &dims_[0]) < 0;
        if (err) {
            throw error("setting chunked dataset layout failed");
        }
    }

private:
    std::vector<hsize_t> dims_;
};

} //namespace storage
} //namespace policy
} //namespace h5xx

#endif // ! H5XX_POLICY_STORAGE_HPP
