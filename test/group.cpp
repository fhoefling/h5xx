/*
 * Copyright © 2010-2014 Felix Höfling
 * Copyright © 2013      Manuel Dibak
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

#define BOOST_TEST_MODULE h5xx_group
#include <boost/test/unit_test.hpp>

#include <h5xx/group.hpp>

#include <test/ctest_full_output.hpp>
#include <test/catch_boost_no_throw.hpp>
#include <test/fixture.hpp>

BOOST_GLOBAL_FIXTURE( ctest_full_output )

namespace fixture { // preferred over BOOST_FIXTURE_TEST_SUITE

char filename[] = "test_h5xx_group.h5";
typedef h5file<filename> BOOST_AUTO_TEST_CASE_FIXTURE;

using namespace h5xx;

BOOST_AUTO_TEST_CASE( construction )
{
    BOOST_CHECK_NO_THROW(group());                 // default constructor

    group foo, bar;

    BOOST_CHECK_NO_THROW(group g(file));           // convert h5xx::file to h5xx::group
    BOOST_CHECK_NO_THROW(group(file, "foo"));      // create group in a file on the fly
    BOOST_CHECK_NO_THROW(foo.open(file, "foo"));   // open existing group in a file
    BOOST_CHECK_EQUAL(get_name(foo), "/foo");
    BOOST_CHECK(foo.valid());

    BOOST_CHECK_NO_THROW(bar.open(foo, "bar"));    // create group in a group
    BOOST_CHECK_EQUAL(get_name(bar), "/foo/bar");
    BOOST_CHECK(bar.valid());

    hid_t hid = foo.hid();
    BOOST_CHECK_NO_THROW(group g(bar));            // copy constructor with move semantics, closes bar
    BOOST_CHECK(!bar.valid());
    BOOST_CHECK_NO_THROW(bar = foo);               // assignment operator with move semantics
    BOOST_CHECK_EQUAL(bar.hid(), hid);
    BOOST_CHECK(!foo.valid());

    group root(file);
//  root = group(file);                            // doesn't compile due to copy construction of temporary group
    BOOST_CHECK_EQUAL(get_name(root), "/");
    BOOST_CHECK_NO_THROW(foo.open(root, "foo"));   // re-open existing and opened group
    BOOST_CHECK(get_name(foo) == get_name(bar));
    BOOST_CHECK(foo.hid() != bar.hid());

    BOOST_CHECK_NO_THROW(foo.close());             // close group
    BOOST_CHECK_NO_THROW(foo.close());             // closing again doesn't throw
}

BOOST_AUTO_TEST_CASE( usage )
{
    group one, two, three, four;
    BOOST_CHECK_NO_THROW(one.open(file, "one"));
    BOOST_CHECK_NO_THROW(three.open(one, "two/three"));  // create intermediate group
    BOOST_CHECK(exists_group(one, "two"));               // check existence of intermediate group
    BOOST_CHECK_NO_THROW(two.open(one, "two"));
    BOOST_CHECK_NO_THROW(four.open(two, "four"));
    BOOST_CHECK(get_name(one) == "/one");
    BOOST_CHECK(get_name(two) == "/one/two");
    BOOST_CHECK(get_name(three) == "/one/two/three");
    BOOST_CHECK(get_name(four) == "/one/two/four");
}

} // namespace fixture
