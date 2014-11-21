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

#ifndef H5XX_HYPERSLAB_HPP
#define H5XX_HYPERSLAB_HPP

//#include <h5xx/hdf5_compat.hpp>
//#include <h5xx/error.hpp>
//#include <h5xx/utility.hpp>
//
//#include <boost/lexical_cast.hpp>
//#include <string>
#include <algorithm>
#include <iterator>
#include <vector>


namespace h5xx {

class hyperslab {
public:
    hyperslab();

    template <class ArrayType>
    hyperslab(ArrayType offset, ArrayType count);

    template <class ArrayType>
    hyperslab(ArrayType offset, ArrayType count, ArrayType stride, ArrayType block);

private:
    std::vector<int> offset_, count_, stride_, block_;
};

template <class ArrayType>
hyperslab::hyperslab(ArrayType offset, ArrayType count)
{
    std::copy(offset.begin(), offset.end(), std::back_inserter(offset_));
    std::copy(count.begin(),  count.end(),  std::back_inserter(count_));
}

template <class ArrayType>
hyperslab::hyperslab(ArrayType offset, ArrayType count, ArrayType stride, ArrayType block)
{
    std::copy(offset.begin(), offset.end(), std::back_inserter(offset_));
    std::copy(count.begin(),  count.end(),  std::back_inserter(count_));
    std::copy(stride.begin(), stride.end(), std::back_inserter(stride_));
    std::copy(block.begin(),  block.end(),  std::back_inserter(block_));
}

} // namespace h5xx


#endif // ! H5XX_HYPERSLAB_HPP
