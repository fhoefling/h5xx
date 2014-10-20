/*
 * Copyright © 2014 Felix Höfling
 * Copyright © 2014 Klaus Reuter
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

#include <algorithm>
#include <iterator>
#include <vector>

#include <h5xx/error.hpp>
#include <h5xx/h5xx.hpp>
#include <h5xx/policy/filter.hpp>

namespace h5xx {
namespace policy {
namespace storage {

/**
 * policy class to specify a contiguous dataset layout
 */
struct contiguous
{
    contiguous() {}

    /** set compact storage for given property list */
    void set_storage(hid_t plist) const
    {
        if (H5Pset_layout(plist, H5D_CONTIGUOUS) < 0) {
            throw error("setting contiguous dataset layout failed");
        }
    }
};

/**
 * policy class to specify a compact dataset layout
 */
struct compact
{
    compact() {}

    /** set compact storage for given property list */
    void set_storage(hid_t plist) const
    {
        if (H5Pset_layout(plist, H5D_COMPACT) < 0) {
            throw error("setting compact dataset layout failed");
        }
    }
};

/**
 * policy class to specify a chunked dataset layout of given size, optionally
 * along with a filter pipeline (for, e.g., data compression).
 */
class chunked
{
public:
    typedef std::vector<h5xx::policy::filter::filter_base> filter_pipeline_t;

    /**
     * Specify the size, in dataset elements, of a chunk in each dimension.
     * The number of dimensions must equal the rank of the dataset.
     */
    chunked(int ndims, hsize_t const* dims)
    {
        std::copy(dims, dims + ndims, std::back_inserter(dims_));
    }

    template <typename ContainerType>
    chunked(ContainerType const& dims)
    {
        std::copy(dims.begin(), dims.end(), std::back_inserter(dims_));
    }

    /** set chunked storage layout for given property list */
    void set_storage(hid_t plist) const
    {
        bool err = false;
        err |= H5Pset_layout(plist, H5D_CHUNKED) < 0;
        err |= H5Pset_chunk(plist, dims_.size(), &*dims_.begin()) < 0;
        if (err) {
            throw error("setting chunked dataset layout failed");
        }

        // add filter pipeline to property list
        typename filter_pipeline_t::const_iterator f;
        for (f = filter_.begin(); f != filter_.end(); ++f) {
            f->set_filter(plist);
        }
    }

    /**
     * add a filter to filter pipeline
     */
    chunked add(typename filter_pipeline_t::value_type const& filter)
    {
        filter_.push_back(filter);
        return *this;
    }

private:
    // chunk dimensions
    std::vector<hsize_t> dims_;

    // filter pipeline
    filter_pipeline_t filter_;
};

} //namespace storage
} //namespace policy
} //namespace h5xx

#endif // ! H5XX_POLICY_STORAGE_HPP
