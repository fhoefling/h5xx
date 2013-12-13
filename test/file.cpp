/*
 * Copyright © 2013 Manuel Dibak
 * Copyright © 2013 Felix Höfling
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

#define BOOST_TEST_MODULE h5xx_file
#include <boost/test/unit_test.hpp>

#include <h5xx/file.hpp>

#include <unistd.h>
#include <test/ctest_full_output.hpp>
#include <test/catch_boost_no_throw.hpp>

using namespace h5xx;

BOOST_GLOBAL_FIXTURE( ctest_full_output )

// test standard use cases
BOOST_AUTO_TEST_CASE( use_cases )
{
    char const* foo = "test_h5xx_file_use_cases.h5";

    file h5file;
    BOOST_CHECK_NO_THROW(h5file.open(foo));
    BOOST_CHECK_THROW(h5file.open(foo), h5xx::error);         // open again
    BOOST_CHECK_NO_THROW(h5file.flush());
    BOOST_CHECK_EQUAL(h5file.name(), std::string(foo));
    BOOST_CHECK_NO_THROW(h5file.close());
    BOOST_CHECK_NO_THROW(h5file.close());                     // close is silent for closed files
    BOOST_CHECK_NO_THROW(h5file.flush());                     // flush is silent for closed files
    BOOST_CHECK(h5file.hid() == -1);
    BOOST_CHECK(is_hdf5_file(foo) > 0);
    unlink(foo);
}

// test opening modes
BOOST_AUTO_TEST_CASE( open )
{
    char const* foo = "test_h5xx_file_opening_modes.h5";

    BOOST_CHECK_NO_THROW(file());                              // default trivial constructor and destructor
//    BOOST_CHECK_NO_THROW(file(foo));                         // warning: this means a local declaration of "foo"
    BOOST_CHECK_NO_THROW(file f(foo));                         // default: in | out, create file
    BOOST_CHECK(is_hdf5_file(foo) > 0);
    BOOST_CHECK_NO_THROW(file f(foo));                         // re-open existing file
    BOOST_CHECK_NO_THROW(file(foo, file::in));                 // read-only
    BOOST_CHECK_NO_THROW(file(foo, file::out));                // append
    BOOST_CHECK_NO_THROW(file(foo, file::out | file::trunc));  // write and truncate

    // remove and recreate file
    unlink(foo);
    BOOST_CHECK_THROW(file(foo, file::in), error);                 // read non-existing file
    BOOST_CHECK_NO_THROW(file(foo, file::out | file::excl));       // create new file
    BOOST_CHECK_THROW(file(foo, file::out | file::excl), error);   // don't overwrite existing file
    unlink(foo);

    // check conflicting modes, should not create a file
    BOOST_CHECK_THROW(file(foo, file::trunc), error);                          // no "out" given
    BOOST_CHECK_THROW(file(foo, file::excl), error);                           // no "out" given
    BOOST_CHECK_THROW(file(foo, file::trunc | file::excl), error);             // "trunc" and "excl" are conflicting
    BOOST_CHECK_THROW(file(foo, file::out | file::trunc | file::excl), error); // "trunc" and "excl" are conflicting

    // file should not exist here
    BOOST_CHECK(is_hdf5_file(foo) < 0);
}

// test hdf5_id
BOOST_AUTO_TEST_CASE( hdf5_id )
{
    char const* foo = "test_h5xx_file_hdf5_id.h5";

    // create file and test HDF5 object ID
    BOOST_CHECK_EQUAL(file().hid(), -1);
    BOOST_CHECK(file(foo).hid() >= 0);
    BOOST_CHECK(is_hdf5_file(foo) > 0);
    unlink(foo);
}

