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
#include <iostream>
#include <cstdio>

const int NI=10;

typedef boost::array<int, NI> array_t;

template <typename ArrayT>
void print_array(ArrayT const& array)
{
    for (unsigned int i = 0; i < array.size(); i++)
    {
        printf("%2d ", array[i]);
    }
    printf("\n");
}


void write_int_data(std::string const& filename, array_t const& array)
{
    h5xx::file file(filename, h5xx::file::trunc);
    std::string name;

    {
        // --- create dataset and fill it with the default array data (positive values)
        name = "integer array";
        h5xx::create_dataset(file, name, array);
        h5xx::write_dataset(file, name, array);

        // --- create a slice object (aka hyperslab) to specify the location in the dataset to be overwritten
        boost::array<int,1> offset; offset[0] = 4;
        boost::array<int,1> count; count[0] = 2;
        h5xx::slice slice(offset, count);

        // --- data to be written to the slice (negative values)
        boost::array<int,2> data = {{-1,-2}};

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
        array_t data;
        // --- read the complete dataset into data
        h5xx::read_dataset(file, name, data);
        printf("original integer array read from file, negative number patch was written using a slice\n");
        print_array(data);
        printf("\n");
    }

    // read and print a subset of the dataset
    {
        boost::array<int,6> data;

        // --- create a slice object (aka hyperslab) to specify the patch to be read from the dataset
        boost::array<int,1> offset; offset[0] = 2;
        boost::array<int,1> count; count[0] = 6;
        h5xx::slice slice(offset, count);

        h5xx::read_dataset(file, name, data, slice);

        printf("1D slice of the integer array, zoom on the negative number patch\n");
        print_array(data);
        printf("\n");
    }
}


int main(int argc, char** argv)
{
    std::string filename = argv[0];
    filename.append(".h5");

    // --- do a few demos/tests using integers
    {
        array_t array;
        for (int i = 0; i < NI; i++)
            array[i] = i;

        write_int_data(filename, array);

        read_int_data(filename);
    }

    return 0;
}
