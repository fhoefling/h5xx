/*
 * Copyright © 2010  Felix Höfling
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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_H5xx
#include <boost/test/unit_test.hpp>

#include <unistd.h>
#include <H5Cpp.h>
#include <H5xx.hpp>

BOOST_AUTO_TEST_CASE( test_H5xx_attribute )
{
    using namespace H5;
    using namespace H5xx;

    char const filename[] = "test_H5xx.hdf5";
    H5File file(filename, H5F_ACC_TRUNC);
    Group group = file.openGroup("/");

    uint64_t uint_value = 9223372036854775783;  // largest prime below 2^63
    attribute(group, "integral, scalar") = 1; // works with 1UL;
    attribute(group, "integral, scalar") = uint_value;  // overwrite value

    // long double is supported by the HDF5 library,
    // but neither by h5dump nor pytables ...
    long double ldouble_value = sqrtl(2.);
    attribute(group, "long double, scalar") = ldouble_value;
    attribute(group, "double, scalar") = static_cast<double>(ldouble_value);

    boost::array<char const*, 3> string_array = {{
        "HALMD", "HAL's MD package",
        "Highly accelerated large-scale molecular dynamics simulation package"
    }};
    attribute(group, "char [], scalar") = string_array[1];
    attribute(group, "string, scalar") = std::string(string_array[1]);
    attribute(group, "char [], array") = string_array;

    typedef boost::array<double, 5> double_array_type;
    double_array_type value_array = {{
        1., sqrt(2.), 2., sqrt(3.), 3.
    }};
    attribute(group, "double, array") = value_array;

    typedef std::vector<double> double_vector_type;
    double_vector_type value_vector(value_array.size());
    std::copy(value_array.begin(), value_array.end(), value_vector.begin());
    attribute(group, "double, std::vector") = value_vector;

    typedef boost::multi_array<int, 3> multi_array3;
    int data3[] = {
        99,98,97,96,
        95,94,93,92,
        91,90,89,88,

        87,86,85,84,
        83,82,81,80,
        79,78,77,76
    };
    multi_array3 value_multi_array(boost::extents[2][3][4]);
    value_multi_array.assign(data3, data3 + 2 * 3 * 4);
    attribute(group, "int, multi_array") = value_multi_array;

    // re-open file
    file.close();
    file.openFile(filename, H5F_ACC_RDONLY);
    group = file.openGroup("/");

    // read attributes
    BOOST_CHECK(attribute(group, "integral, scalar").as<uint64_t>() == uint_value);
    BOOST_CHECK(attribute(group, "long double, scalar").as<long double>() == ldouble_value);
    BOOST_CHECK(attribute(group, "double, scalar").as<double>()
                == static_cast<double>(ldouble_value));
    BOOST_CHECK(attribute(group, "char [], scalar").as<std::string>() == string_array[1]);
    BOOST_CHECK(attribute(group, "string, scalar").as<std::string>() == string_array[1]);
    // TODO read support for string array is missing
//     BOOST_CHECK(attribute(group, "char [], array").as<boost::array<char const*, 3> >() == strings);
    BOOST_CHECK(attribute(group, "double, array").as<double_array_type>() == value_array);
    value_vector = attribute(group, "double, array").as<double_vector_type>();
    BOOST_CHECK(equal(value_vector.begin(), value_vector.end(), value_array.begin()));
    BOOST_CHECK(attribute(group, "int, multi_array").as<multi_array3>() == value_multi_array);

    // remove file
#ifndef NDEBUG
    unlink(filename);
#endif
}

BOOST_AUTO_TEST_CASE( test_H5xx_dataset )
{
    using namespace H5;
    using namespace H5xx;

    char const filename[] = "test_H5xx.hdf5";
    H5File file(filename, H5F_ACC_TRUNC);
    Group group = file.openGroup("/");

    uint64_t uint_value = 9223372036854775783;  // largest prime below 2^63
    DataSet uint_dataset = create_dataset<uint64_t>(group, "dataset, uint");
    H5xx::write(uint_dataset, uint_value);
    H5xx::write(uint_dataset, uint_value);
//     H5xx::make_dataset_writer(group, "dataset, uint", &uint_value)();

    // re-open file
    file.close();
    file.openFile(filename, H5F_ACC_RDONLY);
    group = file.openGroup("/");

    // read datasets
    uint_dataset = group.openDataSet("dataset, uint");
    uint64_t uint_value_;
    read(uint_dataset, &uint_value_, 1);
    BOOST_CHECK(uint_value_ == uint_value);

    // remove file
#ifndef NDEBUG
    unlink(filename);
#endif
}