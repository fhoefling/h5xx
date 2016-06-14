/*
 * Copyright © 2013 Felix Höfling
 * All rights reserved.
 *
 * This file is part of h5xx — a C++ wrapper for the HDF5 library.
 *
 * This software may be modified and distributed under the terms of the
 * 3-clause BSD license.  See accompanying file LICENSE for details.
 */

#include <boost/multi_array.hpp>
#include <h5xx/h5xx.hpp>
#include <iostream>

int main(int argc, char** argv)
{
    // open group within HDF5 file
    if (argc <= 1) {
        std::cout << "Usage: example file.h5" << std::endl;
        return(-1);
    }
    H5::H5File file(argv[1], H5F_ACC_RDONLY);
    H5::Group group = h5xx::open_group(file, "/group");

    // open dataset
    H5::DataSet dataset = group.openDataSet("array");

    //
    // read dataset of rank 3 by iterating over the first index
    //

    // use boost::multi_array to read arrays of rank 2
    typedef boost::multi_array<float, 2> array_type;
    array_type data;

    // determine data set extents assuming an array size of 5×3
    h5xx::read_chunked_dataset(dataset, data, 0);                    // FIXME make shape directly accessible
    array_type::size_type const* shape = data.shape();
    unsigned int n = h5xx::elements(dataset) / (shape[0] * shape[1]);

    std::cout << "Read " << n << " arrays of shape (" << shape[0] << "," << shape[1] << ")" << std::endl;

    for (unsigned int i = 0; i < n; ++i) {
        array_type data;
        h5xx::read_chunked_dataset(dataset, data, i);
        std::cout << "/group/array[" << i << ", 0, 0] = " << data[0][0] << std::endl;
    }

    return 0;
}
