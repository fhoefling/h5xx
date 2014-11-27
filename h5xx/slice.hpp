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

#ifndef H5XX_SLICE_HPP
#define H5XX_SLICE_HPP

#include <algorithm>
#include <iterator>
#include <vector>

namespace h5xx {

class slice {
public:
    slice();

//   --- constructor with zero offset by default?
//    template <class ArrayType>
//    slice(ArrayType count);

    template <class ArrayType>
    slice(ArrayType offset, ArrayType count);

    template <class ArrayType>
    slice(ArrayType offset, ArrayType count, ArrayType stride);

    template <class ArrayType>
    slice(ArrayType offset, ArrayType count, ArrayType stride, ArrayType block);

    int rank() const;

    const std::vector<hsize_t> & get_count() const;
    const std::vector<hsize_t> & get_offset() const;
    const std::vector<hsize_t> & get_stride() const;
    const std::vector<hsize_t> & get_block() const;

private:
    std::vector<hsize_t> offset_, count_, stride_, block_;
};

//template <class ArrayType>
//slice::slice(ArrayType count)
//{
//    std::copy(count.begin(), count.end(), std::back_inserter(count_));
//    offset_.assign(count.size(), 0);
//    stride_.clear();
//    block_.clear();
//}

template <class ArrayType>
slice::slice(ArrayType offset, ArrayType count)
{
    if (offset.size() != count.size())
    {
        throw error("slice specification arrays must have identical length");
    }
    std::copy(offset.begin(), offset.end(), std::back_inserter(offset_));
    std::copy(count.begin(),  count.end(),  std::back_inserter(count_));
    stride_.clear();
    block_.clear();
}

template <class ArrayType>
slice::slice(ArrayType offset, ArrayType count, ArrayType stride)
{
    if ((offset.size() != count.size()) || (count.size() != stride.size()))
    {
        throw error("slice specification arrays must have identical length");
    }
    std::copy(offset.begin(), offset.end(), std::back_inserter(offset_));
    std::copy(count.begin(),  count.end(),  std::back_inserter(count_));
    std::copy(stride.begin(), stride.end(), std::back_inserter(stride_));
    block_.clear();
}

template <class ArrayType>
slice::slice(ArrayType offset, ArrayType count, ArrayType stride, ArrayType block)
{
    if ((offset.size() != count.size()) || (count.size() != stride.size()) || (stride.size() != block.size()))
    {
        throw error("slice specification arrays must have identical length");
    }
    std::copy(offset.begin(), offset.end(), std::back_inserter(offset_));
    std::copy(count.begin(),  count.end(),  std::back_inserter(count_));
    std::copy(stride.begin(), stride.end(), std::back_inserter(stride_));
    std::copy(block.begin(),  block.end(),  std::back_inserter(block_));
}

int slice::rank() const
{
    return count_.size();
}

const std::vector<hsize_t> & slice::get_offset() const
{
    return offset_;
}

const std::vector<hsize_t> & slice::get_count() const
{
    return count_;
}

const std::vector<hsize_t> & slice::get_stride() const
{
    return stride_;
}

const std::vector<hsize_t> & slice::get_block() const
{
    return block_;
}


} // namespace h5xx


#endif // ! H5XX_SLICE_HPP
