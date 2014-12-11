/**
 * Copyright © 2013-2014 Felix Höfling
 * Copyright © 2014      Klaus Reuter
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

#include <h5xx/h5xx.hpp>
#include <boost/multi_array.hpp>
#include <iostream>
#include <vector>
#include <cstdio>

typedef boost::multi_array<int, 2> array_2d_t;
typedef boost::multi_array<int, 1> array_1d_t;
typedef boost::multi_array<double, 2> array_2d_dbl_t;

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


void write_int_data(std::string const& filename, array_2d_t const& array)
{
    h5xx::file file(filename, h5xx::file::trunc);
    std::string name;

    {
        // --- create dataset and fill it with the default array data (positive values)
        name = "integer array";
        h5xx::create_dataset(file, name, array);
        h5xx::write_dataset(file, name, array);

        // --- create a slice object (aka hyperslab) to specify the location in the dataset to be overwritten
        std::vector<int> offset; int offset_raw[2] = {4,4}; offset.assign(offset_raw, offset_raw + 2);
        std::vector<int> count;  int count_raw[2] = {2,2}; count.assign(count_raw, count_raw + 2);
        h5xx::slice slice(offset, count);

        // --- data to be written to the slice (negative values)
        array_1d_t data(boost::extents[4]);
        int data_raw[4] = {-1,-2,-3,-4};
        data.assign(data_raw, data_raw+4);

        // --- overwrite part of the dataset as specified by slice
        h5xx::write_dataset(file, name, data, slice);
    }
}


void read_int_data(std::string const& filename)
{
    h5xx::file file(filename, h5xx::file::in);
    std::string name = "integer array";

    // read and print the full dataset
    {
        array_2d_t array;
        // --- read the complete dataset into array, the array is resized and overwritten internally
        h5xx::read_dataset(file, name, array);
        printf("original integer array read from file, negative number patch was written using a slice\n");
        print_array(array);
        printf("\n");
    }

    // read and print a subset of the dataset
    {
        array_2d_t array;

        // --- create a slice object (aka hyperslab) to specify the patch to be read from the dataset
        std::vector<int> offset; int offset_raw[2] = {3,3}; offset.assign(offset_raw, offset_raw + 2);
        std::vector<int> count;  int count_raw[2] = {4,4}; count.assign(count_raw, count_raw + 2);
        h5xx::slice slice(offset, count);

        // --- allocate memory for the slice (user's responsibility, this is not done internally)
        array.resize(count);

        h5xx::read_dataset(file, name, array, slice);

        printf("2D slice of the integer array, zoom on the negative number patch\n");
        print_array(array);
        printf("\n");
    }

    // read a 2D subset of the dataset into a 1D array
    {
        array_1d_t array;
//        std::cout << array.num_elements() << std::endl; // 0

        // --- create a slice object (aka hyperslab) to specify the patch to be read from the dataset
        std::vector<int> offset; int offset_raw[2] = {3,3}; offset.assign(offset_raw, offset_raw + 2);
        std::vector<int> count;  int count_raw[2] = {4,4}; count.assign(count_raw, count_raw + 2);
        h5xx::slice slice(offset, count);

        // --- allocate memory for the slice
        std::vector<int> total_count;
        total_count.push_back(count[0]*count[1]);
        array.resize(total_count);
//        std::cout << array.num_elements() << std::endl; // 16

        h5xx::read_dataset(file, name, array, slice);

        printf("2D slice of the integer array, copied into a 1D array\n");
        print_array(array);
        printf("\n");
    }
}


int main(int argc, char** argv)
{
    std::string filename = argv[0];
    filename.append(".h5");

    // --- do a few demos/tests using integers
    {
        array_2d_t array(boost::extents[NJ][NI]);
        {
            const int nelem = NI*NJ;
            int data[nelem];
            for (int i = 0; i < nelem; i++)
                data[i] = i;
            array.assign(data, data + nelem);
        }

        write_int_data(filename, array);

        read_int_data(filename);
    }

    return 0;
}
