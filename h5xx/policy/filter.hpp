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

#ifndef H5XX_POLICY_FILTER_HPP
#define H5XX_POLICY_FILTER_HPP

#include <boost/array.hpp>
#include <vector>

#include <h5xx/error.hpp>
#include <h5xx/h5xx.hpp>

namespace h5xx {
namespace policy {
namespace filter {

/**
 * policy class to enable gzip compression of a chunked dataset layout
 *
 * The compression level corresponds to the values of the GNU gzip tool.
 *
 * The filter is optional by default: if the filter result would be larger than
 * the input, then the compression filter returns failure and the uncompressed
 * data is stored in the file.
 */
class deflate
{
public:
    deflate(unsigned int level=6, bool optional=true)
      : level_(level), flags_(optional ? H5Z_FLAG_OPTIONAL : 0)
    {}

    /** set deflate filter for given property list */
    void set_filter(hid_t plist) const
    {
        if (H5Pset_filter(plist, H5Z_FILTER_DEFLATE, flags, 1, &level_) < 0) {
            throw error("setting data compression filter (gzip) failed");
        }
    }

private:
    // filter flags as a bit mask
    unsigned int flags_;
    // compression level
    unsigned int level_;
};

/**
 * policy class to enable SZIP compression of a chunked dataset layout
 *
 * The block size must be even and not greater than 32. This parameter affects
 * the compression ratio; the more the data values vary, the smaller this
 * number should be to achieve better performance. For optimal performance, it
 * is recommended that a chunk's fastest-changing dimension be equal to 128
 * times the block size.
 *
 * The filter is optional by default: if the filter result would be larger than
 * the input, then the compression filter returns failure and the uncompressed
 * data is stored in the file.
 */
class szip
{
public:
    enum coding_t {
        entropy                 /* entropy coding method: best suited for preprocessed data and small numbers   */
      , nearest_neighbour       /* nearest neighbor coding method: preprocess data, then apply entropy coding */
    }

    szip(unsigned int block_size=16, coding_t coding=nearest_neighbour, bool optional=true)
      : flags_(optional ? H5Z_FLAG_OPTIONAL : 0)
    {
        param_[0] = coding;
        param_[1] = block_size;
        if (block_size > 32 || block_size % 2) {
            throw error("SZIP filter: block size must be even and not greater than 32.");
        }
    }

    /** set szip filter for given property list */
    void set_filter(hid_t plist) const
    {
        if (H5Pset_filter(plist, H5Z_FILTER_SZIP, flags, 2, param_) < 0) {
            throw error("setting data compression filter (SZIP) failed");
        }
    }

private:
    // filter flags as a bit mask
    unsigned int flags_;
    // compression parameters
    unsigned int param_[2];
};

/**
 * policy class to set data shuffling filter for a chunked dataset layout
 */
class shuffle
{
public:
    shuffle(bool optional=false)
      : flags_(optional ? H5Z_FLAG_OPTIONAL : 0)
    {}

    /** set data shuffling filter for given property list */
    void set_filter(hid_t plist) const
    {
        if (H5Pset_filter(plist, H5Z_FILTER_SHUFFLE, flags, 0, NULL) < 0) {
            throw error("setting data shuffling filter failed");
        }
    }

private:
    // filter flags as a bit mask
    unsigned int flags_;
};

/**
 * policy class to enable Fletcher32 checksums of a chunked dataset layout
 *
 * The filter can not be made optional.
 */
struct fletcher32
{
    fletcher32() {}

    /** set fletcher32 filter for given property list */
    void set_filter(hid_t plist) const
    {
        if (H5Pset_filter(plist, H5Z_FILTER_FLETCHER32, 0, 0, NULL) < 0) {
            throw error("setting Fletcher32 checksum filter failed");
        }
    }
};

} //namespace filter
} //namespace policy
} //namespace h5xx

#endif // ! H5XX_POLICY_FILTER_HPP
