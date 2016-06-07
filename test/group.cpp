/*
 * Copyright © 2010-2014 Felix Höfling
 * Copyright © 2013      Manuel Dibak
 * All rights reserved.
 *
 * This file is part of h5xx — a C++ wrapper for the HDF5 library.
 *
 * This software may be modified and distributed under the terms of the
 * 3-clause BSD license.  See accompanying file LICENSE for details.
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
    BOOST_CHECK_THROW(group g(bar), h5xx::error);  // copying is not allowed
    BOOST_CHECK_NO_THROW(group g(move(bar)));      // copying from temporary is allowed (with move semantics)
    BOOST_CHECK(!bar.valid());
    BOOST_CHECK_THROW(bar = foo, h5xx::error);     // assignment is not allowed (it makes a copy)
    BOOST_CHECK_NO_THROW(bar = move(foo));         // move assignment
    BOOST_CHECK_EQUAL(bar.hid(), hid);
    BOOST_CHECK(!foo.valid());

    group root(file);
    BOOST_CHECK_EQUAL(get_name(root), "/");
    BOOST_CHECK_NO_THROW(root = group(file));      // convert file to root group
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
