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
        // --- create dataset and fill it with default data (positive values)
        name = "integer array";
        h5xx::create_dataset(file, name, array);
        h5xx::write_dataset(file, name, array);

        // --- create a slice object (hyperslab) to specify the patch to be overwritten in the dataset
        std::vector<int> offset; int offset_raw[2] = {4,4}; offset.assign(offset_raw, offset_raw + 2);
        std::vector<int> count;  int count_raw[2] = {2,2}; count.assign(count_raw, count_raw + 2);
        h5xx::slice slice(offset, count);

        // --- dummy data to be written to the patch
        array_1d_t data(boost::extents[4]);
        int data_raw[4] = {-1,-2,-3,-4};
        data.assign(data_raw, data_raw+4);

        h5xx::write_dataset(file, name, data, slice);
    }

}


void read_int_data(std::string const& filename)
{
    h5xx::file file(filename, h5xx::file::in);
    std::string name = "integer array";

//    // read and print the 2D array w/o modification
//    {
//        array_2d_t array = h5xx::read_dataset<array_2d_t>(file, name);
//        printf("original integer array as read from file, negative number patch was written using a slice\n");
//        print_array(array);
//        printf("\n");
//    }

    // read slice
    {
        array_2d_t array;
        h5xx::read_dataset(file, name, array);
        printf("original integer array as read from file, negative number patch was written using a slice\n");
        print_array(array);
        printf("\n");
    }
//    // (2) select a 2D hyperslab and read it into a 2x2 array
//    // --- offsets and counts for hyperslab selection, can use either std::vector
////    static const hsize_t offset_[] = {1,1};
////    static const hsize_t count_[] = {2,2};
////    std::vector<hsize_t> offset(offset_, offset_ + sizeof(offset_)/sizeof(offset_[0]));;
////    std::vector<hsize_t> count(count_, count_ + sizeof(count_)/sizeof(count_[0]));
//    // --- ... OR boost::arrays
//    boost::array<hsize_t,2> offset = {{1,1}};
//    boost::array<hsize_t,2> count = {{2,2}};
//    {
//        h5xx::dataset dataset(f, name);
//        // create file dataspace from dataset and select hyperslab from the dataset
//        h5xx::dataspace filespace(dataset);
//        filespace.select_hyperslab(offset, count);
//
//        // create memory dataspace
//        boost::array<hsize_t, 2> extents = {{2,2}};
//        h5xx::dataspace memspace(extents);
//
//        array_2d_t array = h5xx::read_dataset<array_2d_t>(dataset, memspace, filespace);
//        printf("hyperslab of the integer array, copied to an array w/ reduced extents\n");
//        print_array(array);
//        printf("\n");
//    }
//
//    // (3) select a 2D hyperslab and read it into a 1D array
//    {
//        h5xx::dataset dataset(f, name);
//        h5xx::dataspace filespace(dataset);
//        filespace.select_hyperslab(offset, count);
//
//        boost::array<hsize_t, 1> extents_1D = {{4}};
//        h5xx::dataspace memspace_1D(extents_1D);
//
//        array_1d_t array = h5xx::read_dataset<array_1d_t>(dataset, memspace_1D, filespace);
//        printf("the same 2D hyperslab of the integer array, copied to a 1D array\n");
//        print_array(array);
//    }
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

        // write array to HDF5 file, do basic tests and simple hyperslab manipulation
        write_int_data(filename, array);

//        // read data back from the HDF5 file and print it (hyperslab has negative sign)
        read_int_data(filename);

//        // write array to HDF5 file, try out some filters and storage modifiers
//        write_int_data_2(filename, array);
    }


//    // --- do a few demos/tests using doubles
//    {
//        array_2d_dbl_t array(boost::extents[NJ][NI]);
//        {
//            const int nelem = NI*NJ;
//            double data[nelem];
//            for (int i = 0; i < nelem; i++)
//                data[i] = double(i);
//            array.assign(data, data + nelem);
//        }
//
//        write_dbl_data(filename, array);
//    }

    return 0;
}
