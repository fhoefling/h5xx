/*
 * Copyright © 2010-2014 Felix Höfling
 * Copyright © 2014-2015 Klaus Reuter
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


#define BOOST_TEST_MODULE h5xx_dataspace

#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>

#include <h5xx/h5xx.hpp>
#include <test/ctest_full_output.hpp>
#include <test/catch_boost_no_throw.hpp>
#include <test/fixture.hpp>

#include <unistd.h>
#include <cmath>
#include <string>

using namespace h5xx;

BOOST_GLOBAL_FIXTURE( ctest_full_output )


namespace fixture { // preferred over BOOST_FIXTURE_TEST_SUITE


template <typename T>
void zero_multi_array(T &array) {
    for (unsigned int i = 0; i < array.num_elements(); i++)
        array.data()[i] = 0;
}

char filename[] = "test_h5xx_dataset.h5";
typedef h5file<filename> BOOST_AUTO_TEST_CASE_FIXTURE;

typedef boost::multi_array<int, 1> array_1d_t;
typedef boost::multi_array<int, 2> array_2d_t;


BOOST_AUTO_TEST_CASE( construction )
{
    BOOST_CHECK_NO_THROW(
        dataspace();
    );

    // --- creation of dataspace from a std::vector
    {
        std::vector<int> vec;
        vec.push_back(2);
        vec.push_back(4);
        vec.push_back(6);
        vec.push_back(8);
        dataspace ds;
        BOOST_CHECK_NO_THROW(
            ds = create_dataspace(vec);
        );
        std::vector<hsize_t> xts;
        BOOST_CHECK_NO_THROW(
            xts = ds.extents();
        );
        BOOST_CHECK_EQUAL(
            xts.size(), size_t(1)
        );
        BOOST_CHECK_EQUAL(
            xts[0], vec.size()
        );
    }

    // --- creation of dataspace from a boost::array
    {
        boost::array<int, 4> vec = {{2,4,6,8}};
        dataspace ds;
        BOOST_CHECK_NO_THROW(
            ds = create_dataspace(vec);
        );
        //boost::array<hsize_t,size_t(1)> xts;
        std::vector<hsize_t> xts;
        BOOST_CHECK_NO_THROW(
            xts = ds.extents();
        );
        BOOST_CHECK_EQUAL(
            xts.size(), size_t(1)
        );
        BOOST_CHECK_EQUAL(
            xts[0], vec.size()
        );
    }

//    --- dataset test code ---
//    BOOST_CHECK_NO_THROW(
//        dataset();                                      // default constructor
//    );
//
//    BOOST_CHECK_NO_THROW(
//            create_dataset<int>(file, "foo");
//    );
//    BOOST_CHECK_NO_THROW(
//            write_dataset(file, "foo", 1);              // create dataset in a file's root group
//    );
//    BOOST_CHECK_NO_THROW(
//            dataset(file, "foo");                       // open existing attribute on-the-fly
//    );
//
//    dataset foo(file, "foo");
//    BOOST_CHECK_EQUAL(get_name(foo), "/foo");          // full path of the dataset
//    BOOST_CHECK(foo.valid());
//
//    // TODO recheck, taken from the attribute unit test program
//    hid_t hid = foo.hid();
//    dataset bar;
//    BOOST_CHECK_THROW(bar = foo, h5xx::error);         // assignment is not allowed (it makes a copy)
//    BOOST_CHECK_NO_THROW(bar = move(foo));             // move assignment
//    BOOST_CHECK_EQUAL(bar.hid(), hid);
//    BOOST_CHECK(!foo.valid());
//
//    // TODO recheck, taken from the attribute unit test program
//    BOOST_CHECK_THROW(dataset g(bar), h5xx::error);  // copying is not allowed
//    BOOST_CHECK_NO_THROW(dataset g(move(bar)));      // copying from temporary is allowed (with move semantics)
//    BOOST_CHECK(!bar.valid());
}


} //namespace fixture
