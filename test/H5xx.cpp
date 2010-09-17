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
    attribute(group, "integral, scalar") = 1;   // store something of wrong type first
    attribute(group, "integral, scalar") = uint_value;  // overwrite value

    // long double is supported by the HDF5 library,
    // but neither by h5dump nor pytables ...
    long double ldouble_value = sqrtl(2.);
    attribute(group, "long double, scalar") = 2;   // store something of wrong type first
    attribute(group, "long double, scalar") = ldouble_value;
    attribute(group, "double, scalar") = static_cast<double>(ldouble_value);

    boost::array<char const*, 3> cstring_array = {{
        "HALMD", "HAL's MD package",
        "Highly accelerated large-scale molecular dynamics simulation package"
    }};
    typedef boost::array<std::string, 3> string_array_type;
    attribute(group, "char [], scalar") = cstring_array[1];
    attribute(group, "string, scalar") = std::string(cstring_array[1]);
    attribute(group, "char [], array") = cstring_array;

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
    multi_array3 multi_array_value(boost::extents[2][3][4]);
    multi_array_value.assign(data3, data3 + 2 * 3 * 4);
    attribute(group, "int, multi_array") = multi_array_value;

    // re-open file
    file.close();
    file.openFile(filename, H5F_ACC_RDONLY);
    group = file.openGroup("/");

    // check has_type<>
    BOOST_CHECK(has_type<uint64_t>(group.openAttribute("integral, scalar")));
    BOOST_CHECK(has_type<long double>(group.openAttribute("long double, scalar")));
    BOOST_CHECK(has_type<double>(group.openAttribute("double, scalar")));
    BOOST_CHECK(has_type<char const*>(group.openAttribute("char [], scalar")));
    BOOST_CHECK(has_type<std::string>(group.openAttribute("string, scalar")));
    BOOST_CHECK(has_type<string_array_type>(group.openAttribute("char [], array")));
    BOOST_CHECK(has_type<double_array_type>(group.openAttribute("double, array")));
    BOOST_CHECK(has_type<double_vector_type>(group.openAttribute("double, std::vector")));
    BOOST_CHECK(has_type<multi_array3>(group.openAttribute("int, multi_array")));

    // check has_scalar_space()
    BOOST_CHECK(has_scalar_space(group.openAttribute("integral, scalar")));
    BOOST_CHECK(has_scalar_space(group.openAttribute("long double, scalar")));
    BOOST_CHECK(has_scalar_space(group.openAttribute("double, scalar")));
    BOOST_CHECK(has_scalar_space(group.openAttribute("char [], scalar")));
    BOOST_CHECK(has_scalar_space(group.openAttribute("string, scalar")));

    // check has_extent<>
    BOOST_CHECK(has_extent<string_array_type>(group.openAttribute("char [], array")));
    BOOST_CHECK(has_extent<double_array_type>(group.openAttribute("double, array")));
    BOOST_CHECK(has_extent<multi_array3>(group.openAttribute("int, multi_array"),
                multi_array_value.shape()));

    // read attributes
    BOOST_CHECK(attribute(group, "integral, scalar").as<uint64_t>() == uint_value);
    BOOST_CHECK(attribute(group, "long double, scalar").as<long double>() == ldouble_value);
    BOOST_CHECK(attribute(group, "double, scalar").as<double>()
                == static_cast<double>(ldouble_value));
    BOOST_CHECK(attribute(group, "char [], scalar").as<std::string>() == cstring_array[1]);
    BOOST_CHECK(attribute(group, "string, scalar").as<std::string>() == cstring_array[1]);
    // read support for fixed-size string array is missing
//     BOOST_CHECK(attribute(group, "char [], array").as<const char*>() == cstring_array);
    BOOST_CHECK(attribute(group, "double, array").as<double_array_type>() == value_array);
    value_vector = attribute(group, "double, array").as<double_vector_type>();
    BOOST_CHECK(equal(value_vector.begin(), value_vector.end(), value_array.begin()));
    BOOST_CHECK(attribute(group, "int, multi_array").as<multi_array3>() == multi_array_value);

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

    //
    // create and write datasets
    //

    // scalar type
    uint64_t uint_value = 9223372036854775783;  // largest prime below 2^63
    H5xx::make_dataset_writer(group, "uint", &uint_value)();
    // overwrite data set
    DataSet uint_dataset = create_dataset<uint64_t>(group, "uint");
    H5xx::write(uint_dataset, uint_value);
    H5xx::write(uint_dataset, uint_value + 1);

    // array type
    typedef boost::array<double, 3> array_type;
    array_type array_value = {{ 1, sqrt(2), 2 }};
    array_type array_value2 = {{ -1, sqrt(3), -3 }};
    DataSet array_dataset
        = create_dataset<array_type>(group, "array", 2);  // fixed size
    H5xx::write(array_dataset, array_value, 0);           // write entry #0
    H5xx::write(array_dataset, array_value2, 1);          // write entry #1

    // multi-array type
    typedef boost::multi_array<int, 2> multi_array2;
    int data2[] = {
        99,98,97,96,
        95,94,93,92,
        91,90,89,88,
    };
    multi_array2 multi_array_value(boost::extents[3][4]);
    multi_array_value.assign(data2, data2 + 3 * 4);
    DataSet multi_array_dataset
        = create_dataset<multi_array2>(group, "multi_array", multi_array_value.shape());
    H5xx::write(multi_array_dataset, multi_array_value);    // append
    multi_array_value[1][2] = 1;
    H5xx::write(multi_array_dataset, multi_array_value);    // append
    multi_array_value[1][2] = 2;
    H5xx::write(multi_array_dataset, multi_array_value, 0);  // overwrite first entry

    // re-open file
    file.flush(H5F_SCOPE_GLOBAL);
    file.close();
    file.openFile(filename, H5F_ACC_RDONLY);
    group = file.openGroup("/");

    //
    // read datasets
    //

    // scalar type dataset
    uint_dataset = group.openDataSet("uint");
    BOOST_CHECK(has_type<uint64_t>(uint_dataset));
    BOOST_CHECK(elements(uint_dataset) == 2);

    uint64_t uint_value_;
    read(uint_dataset, &uint_value_, 0);
    BOOST_CHECK(uint_value_ == uint_value);
    read(uint_dataset, &uint_value_, 1);
    BOOST_CHECK(uint_value_ == uint_value + 1);
    read(uint_dataset, &uint_value_, -1);
    BOOST_CHECK(uint_value_ == uint_value + 1);
    read(uint_dataset, &uint_value_, -2);
    BOOST_CHECK(uint_value_ == uint_value);
    BOOST_CHECK_THROW(read(uint_dataset, &uint_value_, 2), std::runtime_error);
    BOOST_CHECK_THROW(read(uint_dataset, &uint_value_, -3), std::runtime_error);

    // array type dataset
    array_dataset = group.openDataSet("array");
    BOOST_CHECK(has_type<array_type>(array_dataset));
    BOOST_CHECK(has_extent<array_type>(array_dataset, false));
    BOOST_CHECK(elements(array_dataset) == 2 * 3);
    array_type array_value_;
    read(array_dataset, &array_value_, 0);
    BOOST_CHECK(array_value_ == array_value);
    read(array_dataset, &array_value_, 1);
    BOOST_CHECK(array_value_ == array_value2);

    // multi-array type dataset
    multi_array_dataset = group.openDataSet("multi_array");
    BOOST_CHECK(has_type<multi_array2>(multi_array_dataset));
    BOOST_CHECK(has_extent<multi_array2>(multi_array_dataset, multi_array_value.shape(), false));
    BOOST_CHECK(elements(multi_array_dataset) == 2 * 3 * 4);
    multi_array2 multi_array_value_(boost::extents[3][4]);
    read(multi_array_dataset, &multi_array_value_, 0);
    multi_array_value[1][2] = 2;
    BOOST_CHECK(multi_array_value_ == multi_array_value);
    read(multi_array_dataset, &multi_array_value_, 1);
    multi_array_value[1][2] = 1;
    BOOST_CHECK(multi_array_value_ == multi_array_value);

    // remove file
#ifndef NDEBUG
    unlink(filename);
#endif
}