/*
 * Copyright © 2013-2014 Felix Höfling
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

#include <boost/multi_array.hpp>
#include <h5xx/h5xx.hpp>
#include <iostream>

#include <cstdio>

typedef boost::multi_array<int, 2> array_2d_t;
typedef boost::multi_array<int, 1> array_1d_t;

const int NI=10;
const int NJ=NI;

void print_array(array_2d_t const& array)
{
    for (unsigned int j = 0; j < array.shape()[1]; j++)
    {
        for (unsigned int i = 0; i < array.shape()[0]; i++)
        {
            printf("%2d ", array[j][i]);
        }
        printf("\n");
    }
}

void print_array(array_1d_t const& array)
{
    for (unsigned int i = 0; i < array.shape()[0]; i++)
    {
        printf("%2d ", array[i]);
    }
    printf("\n");
}

void write_dataset(std::string const& filename, array_2d_t const& array)
{
    h5xx::file file(filename, h5xx::file::trunc);
    std::string name;

    // (1) write chunked and compressed dataset
    {
        name = "integer array";

        h5xx::policy::dataset_creation_property_list dcpl;
        std::vector<hsize_t> chunk_dims(2, 2);
        dcpl.add( h5xx::policy::storage::chunked(chunk_dims) );
        dcpl.add( h5xx::policy::filter::deflate() );

        // derive dataspace and datatype from the array internally
        h5xx::create_dataset(file, name, array, dcpl /* optional argument */);

        h5xx::write_dataset(file, name, array);
    }

    // (2) write dataset using defaults
    {
        name = "integer array, 2";

        // construct dataspace from a Boost multi_array
        h5xx::dataspace dataspace(array);
        // pull datatype from a Boost multi_array
        h5xx::datatype datatype(array);

        h5xx::create_dataset(file, name, datatype, dataspace);

        h5xx::write_dataset(file, name, array);
    }

    // (3) overwrite part of dataset (1) using a hyperslab
    {
        name = "integer array";

        h5xx::dataset dataset(file, name);
        h5xx::dataspace filespace(dataset);
        boost::array<hsize_t,2> offset = {{4,4}};
        boost::array<hsize_t,2> count = {{2,2}};
        filespace.select_hyperslab(offset, count);

        // construct a 2x2 array and fill it with negative numbers
        boost::array<size_t, 2> hyperslab_extents = {{2,2}};
        array_2d_t hyperslab_data(hyperslab_extents);
        const int nelem = 4;
        int data[nelem];
        for (int i = 0; i < nelem; i++)
            data[i] = -1*(i+1);
        hyperslab_data.assign(data, data + nelem);
        h5xx::dataspace memspace = h5xx::create_dataspace(hyperslab_data);

        h5xx::write_dataset(dataset, hyperslab_data, memspace, filespace);
    }
}

void read_dataset(std::string const& filename)
{
    h5xx::file f(filename, h5xx::file::in);
    std::string name = "integer array";

    // (1) read and print the 2D array w/o modification
    {
        array_2d_t array = h5xx::read_dataset<array_2d_t>(f, name);
        printf("original integer array as read from file\n");
        print_array(array);
        printf("\n");
    }

    // (2) select a 2D hyperslab and read it into a 2x2 array
    // --- offsets and counts for hyperslab selection, can use either std::vector
//    static const hsize_t offset_[] = {1,1};
//    static const hsize_t count_[] = {2,2};
//    std::vector<hsize_t> offset(offset_, offset_ + sizeof(offset_)/sizeof(offset_[0]));;
//    std::vector<hsize_t> count(count_, count_ + sizeof(count_)/sizeof(count_[0]));
    // --- ... OR boost::arrays
    boost::array<hsize_t,2> offset = {{1,1}};
    boost::array<hsize_t,2> count = {{2,2}};
    {
        h5xx::dataset dataset(f, name);
        // create file dataspace from dataset and select hyperslab from the dataset
        h5xx::dataspace filespace(dataset);
        filespace.select_hyperslab(offset, count);

        // create memory dataspace
        boost::array<hsize_t, 2> extents = {{2,2}};
        h5xx::dataspace memspace(extents);

        array_2d_t array = h5xx::read_dataset<array_2d_t>(dataset, memspace, filespace);
        printf("hyperslab of the integer array, copied to an array w/ reduced extents\n");
        print_array(array);
        printf("\n");
    }

    // (3) select a 2D hyperslab and read it into a 1D array
    {
        h5xx::dataset dataset(f, name);
        h5xx::dataspace filespace(dataset);
        filespace.select_hyperslab(offset, count);

        boost::array<hsize_t, 1> extents_1D = {{4}};
        h5xx::dataspace memspace_1D(extents_1D);

        array_1d_t array = h5xx::read_dataset<array_1d_t>(dataset, memspace_1D, filespace);
        printf("the same 2D hyperslab of the integer array, copied to a 1D array\n");
        print_array(array);
    }
}

int main(int argc, char** argv)
{
    std::string filename = argv[0];
    filename.append(".h5");

    // set-up data as 2D Boost.MultiArray
    array_2d_t array(boost::extents[NJ][NI]);
    const int nelem = NI*NJ;
    int data[nelem];
    for (int i = 0; i < nelem; i++)
        data[i] = i;
    array.assign(data, data + nelem);

    // write array to HDF5 file
    write_dataset(filename, array);

    // read data from HDF5 file, test hyperslab selection
    read_dataset(filename);

    return 0;
}
