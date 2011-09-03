/*
 * Copyright © 2010  Felix Höfling
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

#define BOOST_TEST_MODULE h5xx_dataset
#include <boost/test/unit_test.hpp>

#include <cmath>
#include <h5xx/h5xx.hpp>
#include <unistd.h>

BOOST_AUTO_TEST_CASE( h5xx_dataset )
{
    char const filename[] = "test_h5xx.hdf5";
    H5::H5File file(filename, H5F_ACC_TRUNC);
    H5::Group group = h5xx::open_group(file, "/");

    //
    // create and write datasets
    //

    // scalar type
    uint64_t uint_value = 9223372036854775783LLU;  // largest prime below 2^63
    h5xx::write_dataset(group, "uint", uint_value);
    // overwrite data set
    H5::DataSet uint_dataset = h5xx::create_dataset<uint64_t>(group, "uint");
    h5xx::write_dataset(uint_dataset, uint_value + 1);
    h5xx::write_dataset(uint_dataset, uint_value);

    // array type
    typedef boost::array<double, 3> array_type;
    array_type array_value = {{ 1, std::sqrt(2.), 2 }};
    H5::DataSet array_dataset
        = h5xx::create_dataset<array_type>(group, "array");
    h5xx::write_dataset(array_dataset, array_value);

    // multi-array type
    typedef boost::multi_array<int, 2> multi_array2;
    int data2[] = {
        99,98,97,96,
        95,94,93,92,
        91,90,89,88,
    };
    multi_array2 multi_array_value(boost::extents[3][4]);
    multi_array_value.assign(data2, data2 + 3 * 4);
    H5::DataSet multi_array_dataset
        = h5xx::create_dataset<multi_array2>(group, "multi_array", multi_array_value.shape());
    h5xx::write_dataset(multi_array_dataset, multi_array_value);    // write
    multi_array_value[1][2] = 1;
    h5xx::write_dataset(multi_array_dataset, multi_array_value);    // overwrite

    // vector of scalars
    std::vector<int> int_vector_value(data2, data2 + 3 * 4);
    H5::DataSet int_vector_dataset
            = h5xx::create_dataset<std::vector<int> >(group, "int_vector", int_vector_value.size());
    h5xx::write_dataset(int_vector_dataset, int_vector_value);

    // vector of arrays
    std::vector<array_type> array_vector_value;
    array_type array_value2 = {{ -1, std::sqrt(3.), -3 }};
    array_vector_value.push_back(array_value);
    array_vector_value.push_back(array_value2);
    H5::DataSet array_vector_dataset
            = h5xx::create_dataset<std::vector<array_type> >(group, "array_vector", array_vector_value.size());
    h5xx::write_dataset(array_vector_dataset, array_vector_value);
    // write vector of wrong size
    array_vector_value.push_back(array_value2);
    BOOST_CHECK_THROW(h5xx::write_dataset(array_vector_dataset, array_vector_value), std::runtime_error);
    array_vector_value.pop_back();

    // write to dataset of wrong type and size
    BOOST_CHECK_THROW(h5xx::write_dataset(int_vector_dataset, array_vector_value), std::runtime_error);

    // re-open file
    file.flush(H5F_SCOPE_GLOBAL);
    file.close();
    file.openFile(filename, H5F_ACC_RDONLY);
    group = h5xx::open_group(file, "/");

    //
    // read datasets
    //

    // scalar type dataset
    uint_dataset = group.openDataSet("uint");
    BOOST_CHECK(h5xx::has_type<uint64_t>(uint_dataset));
    BOOST_CHECK(h5xx::elements(uint_dataset) == 1);

    uint64_t uint_value_;
    h5xx::read_dataset(uint_dataset, uint_value_);
    BOOST_CHECK(uint_value_ == uint_value);
    h5xx::read_dataset(file, "uint", uint_value_);
    BOOST_CHECK(uint_value_ == uint_value);

    // array type dataset
    array_dataset = group.openDataSet("array");
    BOOST_CHECK(h5xx::has_type<array_type>(array_dataset));
    BOOST_CHECK(h5xx::has_extent<array_type>(array_dataset));
    BOOST_CHECK(h5xx::elements(array_dataset) == 3);
    array_type array_value_;
    h5xx::read_dataset(array_dataset, array_value_);
    BOOST_CHECK(array_value_ == array_value);

    // read array type dataset as float
    typedef boost::array<float, 3> float_array_type;
    float_array_type float_array_value_;
    h5xx::read_dataset(array_dataset, float_array_value_);
    for (unsigned i = 0; i < array_value.size(); ++i) {
        BOOST_CHECK(float_array_value_[i] == static_cast<float>(array_value[i]));
    }

    // multi-array type dataset
    multi_array_dataset = group.openDataSet("multi_array");
    BOOST_CHECK(h5xx::has_type<multi_array2>(multi_array_dataset));
    BOOST_CHECK(h5xx::has_extent<multi_array2>(multi_array_dataset, multi_array_value.shape()));
    BOOST_CHECK(h5xx::elements(multi_array_dataset) == 3 * 4);
    multi_array2 multi_array_value_;
    h5xx::read_dataset(multi_array_dataset, multi_array_value_);
    BOOST_CHECK(multi_array_value_ == multi_array_value);

    // read multi-array type dataset as char (8 bit)
    typedef boost::multi_array<char, 2> char_multi_array2;
    char_multi_array2 char_multi_array_value_;
    h5xx::read_dataset(multi_array_dataset, char_multi_array_value_);
    for (unsigned i = 0; i < multi_array_value.num_elements(); ++i) {
        BOOST_CHECK(char_multi_array_value_.data()[i] == static_cast<char>(multi_array_value.data()[i]));
    }

    // vector of scalars
    int_vector_dataset = group.openDataSet("int_vector");
    std::vector<int> int_vector_value_;
    h5xx::read_dataset(int_vector_dataset, int_vector_value_);
    BOOST_CHECK(int_vector_value_.size() == int_vector_value.size());
    BOOST_CHECK(std::equal(
        int_vector_value_.begin()
      , int_vector_value_.end()
      , int_vector_value.begin()
    ));

    // read vector of int scalars as short integers
    std::vector<short int> short_vector_value_;
    h5xx::read_dataset(int_vector_dataset, short_vector_value_);
    BOOST_CHECK(short_vector_value_.size() == int_vector_value.size());
    for (unsigned i = 0; i < int_vector_value.size(); ++i) {
        BOOST_CHECK(short_vector_value_[i] == static_cast<short int>(int_vector_value[i]));
    }

    // vector of arrays
    array_vector_dataset = group.openDataSet("array_vector");
    std::vector<array_type> array_vector_value_;
    h5xx::read_dataset(array_vector_dataset, array_vector_value_);
    h5xx::read_dataset(group, "array_vector", array_vector_value_); // use helper function
    BOOST_CHECK(array_vector_value_.size() == array_vector_value.size());
    BOOST_CHECK(std::equal(
        array_vector_value_.begin()
      , array_vector_value_.end()
      , array_vector_value.begin()
    ));

    // read vector of double arrays as float arrays
    std::vector<float_array_type> float_array_vector_value_;
    h5xx::read_dataset(array_vector_dataset, float_array_vector_value_);
    BOOST_CHECK(float_array_vector_value_.size() == array_vector_value.size());
    for (unsigned i = 0; i < array_vector_value.size(); ++i) {
        for (unsigned j = 0; j < float_array_type::static_size; ++j) {
            BOOST_CHECK(float_array_vector_value_[i][j] == static_cast<float>(array_vector_value[i][j]));
        }
    }

    // remove file
#ifndef NDEBUG
//     unlink(filename);
#endif
}
