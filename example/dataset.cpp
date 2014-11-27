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

#include <boost/multi_array.hpp>
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

    // (1) create and write chunked and compressed dataset
    {
        name = "integer array";
        hsize_t chunk_dims[] = {2, 2};
        // derive dataspace and datatype from the array internally
        h5xx::create_dataset(file, name, array
          , h5xx::policy::storage::chunked(2, chunk_dims)  /* optional argument */
                .add(h5xx::policy::filter::deflate())
        );
        h5xx::write_dataset(file, name, array);
    }

    // (2) create and write a dataset, using default settings,
    //     explicitly derive dataspace and datatype from input arrray
    {
        name = "integer array, 2";
        // construct dataspace from a Boost multi_array
        h5xx::dataspace dataspace = h5xx::create_dataspace(array);
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

void write_int_data_2(std::string const& filename, array_2d_t const& array)
{
    h5xx::file file(filename, h5xx::file::out);
    std::string name;

    {
        name = "A -- integer array, compact";

        h5xx::create_dataset(file, name, array
          , h5xx::policy::storage::compact()
        );

        h5xx::write_dataset(file, name, array);
    }

    {
        name = "B -- integer array, contiguous";

        h5xx::create_dataset(file, name, array
          , h5xx::policy::storage::contiguous()
        );

        h5xx::write_dataset(file, name, array);
    }

    {
        name = "C -- integer array, compact, fill_value";
        h5xx::create_dataset(file, name, array
          , h5xx::policy::storage::compact()
                .set(h5xx::policy::storage::fill_value(int(42)))
        );
//        h5xx::write_dataset(file, name, array);
    }

    {
        name = "D -- integer array, compact, track_times";
        h5xx::create_dataset(file, name, array
          , h5xx::policy::storage::compact()
                .set(h5xx::policy::storage::track_times())
        );
        h5xx::write_dataset(file, name, array);
    }

    {
        name = "E -- integer array, chunked, fill_value, deflate";
        std::vector<hsize_t> chunk_dims(2,2);
        h5xx::create_dataset(file, name, array
          , h5xx::policy::storage::chunked(chunk_dims)
                .set(h5xx::policy::storage::fill_value(42))
                .add(h5xx::policy::filter::deflate())
        );
//        h5xx::write_dataset(file, name, array);
    }

    {
        name = "F -- integer array, chunked, shuffle";
        std::vector<hsize_t> chunk_dims(2,2);
        h5xx::create_dataset(file, name, array
          , h5xx::policy::storage::chunked(chunk_dims)
                .add(h5xx::policy::filter::shuffle())
        );
        h5xx::write_dataset(file, name, array);
    }

    {
        name = "G -- integer array, chunked, fletcher32";
        std::vector<hsize_t> chunk_dims(2,2);
        h5xx::create_dataset(file, name, array
          , h5xx::policy::storage::chunked(chunk_dims)
                .add(h5xx::policy::filter::fletcher32())
        );
        h5xx::write_dataset(file, name, array);
    }

    {
        name = "H -- integer array, chunked, scaleoffset";
        std::vector<hsize_t> chunk_dims(2,2);
        h5xx::create_dataset(file, name, array
          , h5xx::policy::storage::chunked(chunk_dims)
                .add(h5xx::policy::filter::scaleoffset<int>())
        );
        h5xx::write_dataset(file, name, array);
    }

    {
        name = "I -- integer array, chunked, nbit";
        std::vector<hsize_t> chunk_dims(2,2);
        h5xx::create_dataset(file, name, array
          , h5xx::policy::storage::chunked(chunk_dims)
                .add(h5xx::policy::filter::nbit())
        );
        h5xx::write_dataset(file, name, array);
    }
}

void read_int_data(std::string const& filename)
{
    h5xx::file file(filename, h5xx::file::in);
    std::string name = "integer array";

    // (1) read and print the 2D array w/o modification
    {
        array_2d_t array;
        h5xx::read_dataset(file, name, array);
        printf("original integer array as read from file, negative number patch was written using a hyperslab\n");
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
        h5xx::dataset dataset(file, name);
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
        h5xx::dataset dataset(file, name);
        h5xx::dataspace filespace(dataset);
        filespace.select_hyperslab(offset, count);

        boost::array<hsize_t, 1> extents_1D = {{4}};
        h5xx::dataspace memspace_1D(extents_1D);

        array_1d_t array = h5xx::read_dataset<array_1d_t>(dataset, memspace_1D, filespace);
        printf("the same 2D hyperslab of the integer array, copied to a 1D array\n");
        print_array(array);
    }
}


void write_dbl_data(std::string const& filename, array_2d_dbl_t const& array)
{
    h5xx::file file(filename, h5xx::file::out);
    std::string name;

    {
        name = "A -- double array, compact";

        h5xx::create_dataset(file, name, array
          , h5xx::policy::storage::compact()
        );

        h5xx::write_dataset(file, name, array);
    }

    {
        name = "B -- double array, contiguous";

        h5xx::create_dataset(file, name, array
          , h5xx::policy::storage::contiguous()
        );

        h5xx::write_dataset(file, name, array);
    }

    {
        name = "C -- double array, compact, fill_value";
        h5xx::create_dataset(file, name, array
          , h5xx::policy::storage::compact()
                .set(h5xx::policy::storage::fill_value(42.667))
        );
//        h5xx::write_dataset(file, name, array);
    }

    {
        name = "D -- double array, compact, track_times";
        h5xx::create_dataset(file, name, array
          , h5xx::policy::storage::compact()
                .set(h5xx::policy::storage::track_times())
        );
        h5xx::write_dataset(file, name, array);
    }

    {
        name = "E -- double array, chunked, fill_value, deflate";
        std::vector<hsize_t> chunk_dims(2,2);
        h5xx::create_dataset(file, name, array
          , h5xx::policy::storage::chunked(chunk_dims)
                .set(h5xx::policy::storage::fill_value(42.667))
                .add(h5xx::policy::filter::deflate())
        );
//        h5xx::write_dataset(file, name, array);
    }

    {
        name = "F -- double array, chunked, shuffle";
        std::vector<hsize_t> chunk_dims(2,2);
        h5xx::create_dataset(file, name, array
          , h5xx::policy::storage::chunked(chunk_dims)
                .add(h5xx::policy::filter::shuffle())
        );
        h5xx::write_dataset(file, name, array);
    }

    {
        name = "G -- double array, chunked, fletcher32";
        std::vector<hsize_t> chunk_dims(2,2);
        h5xx::create_dataset(file, name, array
          , h5xx::policy::storage::chunked(chunk_dims)
                .add(h5xx::policy::filter::fletcher32())
        );
        h5xx::write_dataset(file, name, array);
    }

    {
        name = "H -- double array, chunked, scaleoffset";
        std::vector<hsize_t> chunk_dims(2,2);
        h5xx::create_dataset(file, name, array
          , h5xx::policy::storage::chunked(chunk_dims)
                .add(h5xx::policy::filter::scaleoffset<double>(1))
        );
        h5xx::write_dataset(file, name, array);
    }

    {
        name = "I -- double array, chunked, nbit";
        std::vector<hsize_t> chunk_dims(2,2);
        h5xx::create_dataset(file, name, array
          , h5xx::policy::storage::chunked(chunk_dims)
                .add(h5xx::policy::filter::nbit())
        );
        h5xx::write_dataset(file, name, array);
    }

    {
        name = "J -- double array, (default), fill_value";
        h5xx::create_dataset(file, name, array
          , h5xx::policy::storage::fill_value(42.667)
        );
//        h5xx::write_dataset(file, name, array);
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

        // write array to HDF5 file, do basic tests and simple hyperslab manipulation
        write_int_data(filename, array);

        // read data back from the HDF5 file and print it (hyperslab has negative sign)
        read_int_data(filename);

        // write array to HDF5 file, try out some filters and storage modifiers
        write_int_data_2(filename, array);
    }


    // --- do a few demos/tests using doubles
    {
        array_2d_dbl_t array(boost::extents[NJ][NI]);
        {
            const int nelem = NI*NJ;
            double data[nelem];
            for (int i = 0; i < nelem; i++)
                data[i] = double(i);
            array.assign(data, data + nelem);
        }

        write_dbl_data(filename, array);
    }

    return 0;
}
