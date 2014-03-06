/*
 * Copyright © 2014 Felix Höfling
 * Copyright © 2014 Manuel Dibak
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

#ifndef H5XX_DATASPACE_HPP
#define H5XX_DATASPACE_HPP

#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>

#include <h5xx/hdf5_compat.hpp>
#include <h5xx/error.hpp>
#include <h5xx/utility.hpp>

namespace h5xx {

/**
 * Represents an HDF5 dataspace.
 *
 */
class dataspace
{
public:
    /** default constructor */
    dataspace() : hid_(-1) {}

    /** construct dataspace from HDF5 handle */
    dataspace(hid_t hid) : hid_(hid) {}

    /** construct dataspace of zero rank */
    dataspace(H5S_class_t type);

    /** construct simple dataspace of given extents */
    template <std::size_t N>
    dataspace(boost::array<hsize_t, N> const& dims);

    /** construct simple dataspace of given extents and maximal extents */
    template <std::size_t N>
    dataspace(boost::array<hsize_t, N> const& dims, boost::array<hsize_t, N> const& max_dims);

    /** copy constructor with move semantics
     *
     * The dataspace object on the right hand side is empty after assignment. In
     * C++98, moving from an r-values (e.g., a temporary) is not possible.
     */
    dataspace(dataspace& other);

    /** assignment operator with move semantics
     *
     * the dataspace object on the right hand side is empty after assignment
     */
    dataspace const& operator=(dataspace other);

    /** default destructor */
    ~dataspace();

    /** return HDF5 object ID */
    hid_t hid() const
    {
        return hid_;
    }

    /** returns true if associated to a valid HDF5 object */
    bool valid() const
    {
        return hid_ >= 0;
    }

    int rank() const;

    template <std::size_t N>
    boost::array<hsize_t, N> get_extent(hsize_t *maxdims = NULL) const;

    bool is_scalar() const;

private:
    hid_t hid_;
};

dataspace::dataspace(dataspace& other)
{
    hid_ = other.hid_;
    other.hid_ = -1;
}

dataspace::~dataspace()
{
    if (hid_ >= 0) {
        if(H5Sclose(hid_) < 0){
            throw error("closing h5xx::dataspace with ID " + boost::lexical_cast<std::string>(hid_));
        }
    }
}

dataspace const& dataspace::operator=(dataspace other)
{
    // swap(hid_, other.hid_),
    // the previous dataspace object will be closed upon destruction of other
    hid_t tmp = hid_;
    hid_ = other.hid_;
    other.hid_ = tmp;
    return *this;
}

dataspace::dataspace(H5S_class_t type)
{
    if((hid_ = H5Screate(type)) < 0){
        throw error("creating dataspace");
    }
}

template <std::size_t N>
dataspace::dataspace(boost::array<hsize_t, N> const& dims)
{
    dataspace(dims, dims);
}

template <std::size_t N>
dataspace::dataspace(boost::array<hsize_t, N> const& dims, boost::array<hsize_t, N> const& max_dims)
{
    if ((hid_ = H5Screate_simple(N, &*dims.begin(), &*max_dims.begin())) < 0) {
        throw error("creating simple dataspace");
    }
}

int dataspace::rank() const
{
    int rank = H5Sget_simple_extent_ndims(hid_);
    if (rank < 0) {
        throw error("dataspace has invalid rank");
    }
    return rank;
}

template <std::size_t N>
boost::array<hsize_t, N> dataspace::get_extent(hsize_t *maxdims) const
{
    boost::array<hsize_t, N> dims;
    if (rank() != N) {
        throw error("mismatching dataspace rank");
    }
    if (H5Sget_simple_extent_dims(hid_, &*dims.begin(), maxdims) < 0) {
        throw error("determining extents");
    }
    return dims;
}

bool dataspace::is_scalar() const
{
    return H5Sget_simple_extent_type(hid_) == H5S_SCALAR;
}


} // namespace h5xx

#endif /* ! H5XX_DATASPACE_HPP */
