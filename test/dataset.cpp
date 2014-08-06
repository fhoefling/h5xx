/*
 * Copyright © 2010-2013  Felix Höfling
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
#include <boost/shared_ptr.hpp>

#include <h5xx/h5xx.hpp>
#include <test/ctest_full_output.hpp>
#include <test/catch_boost_no_throw.hpp>
#include <test/fixture.hpp>

#include <unistd.h>
#include <cmath>
#include <string>

BOOST_GLOBAL_FIXTURE( ctest_full_output )

namespace fixture { // preferred over BOOST_FIXTURE_TEST_SUITE

char filename[] = "test_h5xx_dataset.h5";
typedef h5file<filename> BOOST_AUTO_TEST_CASE_FIXTURE;

using namespace h5xx;


BOOST_AUTO_TEST_CASE( construction )
{
    BOOST_CHECK_NO_THROW( dataset() );                 // default constructor

    write_dataset(file, "foo", 1);                     // create dataset in a file's root group
    BOOST_CHECK_NO_THROW(dataset(file, "foo"));        // open existing attribute on-the-fly

    dataset foo(file, "foo");
    BOOST_CHECK_EQUAL(get_name(foo), "/foo");          // full path of the dataset
    BOOST_CHECK_EQUAL(foo.name(), "/foo");             // full path of the dataset
    BOOST_CHECK(foo.valid());

    // TODO recheck, taken from the attribute unit test program
    hid_t hid = foo.hid();
    dataset bar;
    BOOST_CHECK_THROW(bar = foo, h5xx::error);         // assignment is not allowed (it makes a copy)
    BOOST_CHECK_NO_THROW(bar = move(foo));             // move assignment
    BOOST_CHECK_EQUAL(bar.hid(), hid);
    BOOST_CHECK(!foo.valid());

    // TODO recheck, taken from the attribute unit test program
    BOOST_CHECK_THROW(dataset g(bar), h5xx::error);  // copying is not allowed
    BOOST_CHECK_NO_THROW(dataset g(move(bar)));      // copying from temporary is allowed (with move semantics)
    BOOST_CHECK(!bar.valid());
}

BOOST_AUTO_TEST_CASE( scalar_fundamental )
{
    bool bool_value = true;
    std::string bool_name = "bool, scalar";
    BOOST_CHECK_NO_THROW(
            write_dataset(file, bool_name, bool_value)
    );
    BOOST_CHECK_NO_THROW(
            read_dataset<bool>(file, bool_name)
    );
    BOOST_CHECK(
            read_dataset<bool>(file, bool_name) == bool_value
    );
    BOOST_CHECK(
            exists_dataset(file, bool_name) == true
    );
    H5E_BEGIN_TRY{
        BOOST_CHECK_THROW(read_attribute<bool>(file, "X"+bool_name), h5xx::error);
    } H5E_END_TRY

    double double_value = std::sqrt(2.L);
    std::string double_name = "double, scalar";
//    BOOST_CHECK_NO_THROW(
//            write_dataset(file, double_name, bool_value)  // cannot change datatype of dataset
//    );
    BOOST_CHECK_NO_THROW(
            write_dataset(file, double_name, double_value)  // write double
    );
    BOOST_CHECK_NO_THROW(
            write_dataset(file, double_name, 0.5*double_value)  // overwrite double
    );
    BOOST_CHECK_NO_THROW(
            read_dataset<double>(file, double_name)
    );
    BOOST_CHECK(
            exists_dataset(file, double_name) == true
    );

    uint64_t uint64_value = 9223372036854775783LLU;  // largest prime below 2^63
    std::string uint64_name = "uint64, scalar";
    BOOST_CHECK_NO_THROW(
            write_dataset(file, uint64_name, uint64_value)
    );
    BOOST_CHECK_NO_THROW(
            read_dataset<uint64_t>(file, uint64_name)
    );
    BOOST_CHECK(
            exists_dataset(file, uint64_name) == true
    );
}

BOOST_AUTO_TEST_CASE( boost_multi_array)
{
    typedef boost::multi_array<int, 3> multi_array3;
    int data3[] = {99,98,97,96,95,94,93,92,91,90,89,88,87,86,85,84,83,82,81,80,79,78,77,76};
    multi_array3 multi_array_value(boost::extents[2][3][4]);
    multi_array_value.assign(data3, data3 + 2 * 3 * 4);
    multi_array3 read(boost::extents[2][3][4]);

    BOOST_CHECK_NO_THROW(
            write_dataset(file, "boost multi array, int", multi_array_value)
    );
    BOOST_CHECK_NO_THROW(
            read = read_dataset<multi_array3>(file, "boost multi array, int")
    );
    BOOST_CHECK(
            read == multi_array_value
    );
}

/* --- unit tests for old version of dataset ---
BOOST_AUTO_TEST_CASE( h5xx_dataset )
{
    // store H5File object in shared_ptr to destroy it before re-opening the file
    char const filename[] = "test_h5xx_dataset.hdf5";
    boost::shared_ptr<H5::H5File> file(new H5::H5File(filename, H5F_ACC_TRUNC));
    H5::Group group = h5xx::open_group(*file, "/");

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
    file.reset(new H5::H5File(filename, H5F_ACC_RDONLY));
    group = h5xx::open_group(*file, "/");

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
    h5xx::read_dataset(*file, "uint", uint_value_);
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
#ifdef NDEBUG
    file.reset();
    unlink(filename);
#endif
}
--- end old unit tests --- */

} //namespace fixture
