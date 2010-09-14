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

#ifndef HALMD_UTIL_H5XX_DATASET_HPP
#define HALMD_UTIL_H5XX_DATASET_HPP

#define H5E_auto_t_vers 2
#include <H5Cpp.h>

#include <H5xx/attribute.hpp>

namespace H5xx
{

/**
 * HDF5 dataset
 */
class dataset : public H5::DataSet
{
public:
    dataset() {}
    dataset(H5::DataSet const& node) : H5::DataSet(node) {}

    /**
     * returns existing or creates attribute
     */
    attribute operator[](char const* name) const
    {
        return attribute(*this, name);
    }
};

} // namespace H5xx

#endif /* ! HALMD_UTIL_H5XX_DATASET_HPP */
