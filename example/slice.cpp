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

//        // write array to HDF5 file, do basic tests and simple hyperslab manipulation
//        write_int_data(filename, array);
//
//        // read data back from the HDF5 file and print it (hyperslab has negative sign)
//        read_int_data(filename);
//
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
