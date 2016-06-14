/*
 * Copyright © 2010-2013  Felix Höfling
 * All rights reserved.
 *
 * This file is part of h5xx — a C++ wrapper for the HDF5 library.
 *
 * This software may be modified and distributed under the terms of the
 * 3-clause BSD license.  See accompanying file LICENSE for details.
 */

#define BOOST_TEST_MODULE h5xx_group
#include <boost/test/unit_test.hpp>

#include <h5xx/h5xx.hpp>

#include <cmath>
#include <unistd.h>

#include <test/ctest_full_output.hpp>

BOOST_GLOBAL_FIXTURE( ctest_full_output );

BOOST_AUTO_TEST_CASE( h5xx_group )
{
    char const filename[] = "test_h5xx_group.hdf5";
    H5::H5File file(filename, H5F_ACC_TRUNC);

    BOOST_CHECK_NO_THROW(h5xx::open_group(file, "/"));
    BOOST_CHECK_THROW(h5xx::open_group(file, ""), h5xx::error);

    H5::Group group = h5xx::open_group(file, "/");
    BOOST_CHECK_NO_THROW(h5xx::open_group(group, "/"));
    BOOST_CHECK_THROW(h5xx::open_group(group, ""), h5xx::error);

    // create a hierarchy with attributes
    h5xx::write_attribute(h5xx::open_group(file, "/"), "level", 0);
    h5xx::write_attribute(h5xx::open_group(file, "/one"), "level", 1);
    h5xx::write_attribute(h5xx::open_group(file, "/one/two"), "level", 2);
    h5xx::write_attribute(h5xx::open_group(file, "/one/two/three"), "level", 3);

    group = h5xx::open_group(file, "one");
    BOOST_CHECK(group.getNumAttrs() == 1);
    BOOST_CHECK(h5xx::exists_attribute(group, "level"));
    BOOST_CHECK(h5xx::read_attribute<int>(group, "level") == 1);
    BOOST_CHECK(boost::any_cast<int>(h5xx::read_attribute_if_exists<int>(group, "level")) == 1);

    h5xx::open_group(group, "branch");        // create branch in '/one'
    BOOST_CHECK(group.getNumAttrs() == 1);
    BOOST_CHECK(group.getNumObjs() == 2);

    group = h5xx::open_group(group, "two/three/");
    BOOST_CHECK(group.getNumAttrs() == 1);
    BOOST_CHECK(h5xx::exists_attribute(group, "level"));
    BOOST_CHECK(h5xx::read_attribute<int>(group, "level") == 3);
    BOOST_CHECK(boost::any_cast<int>(h5xx::read_attribute_if_exists<int>(group, "level")) == 3);

    group = h5xx::open_group(file, "one/two");
    BOOST_CHECK(group.getNumAttrs() == 1);
    BOOST_CHECK(h5xx::exists_attribute(group, "level"));
    BOOST_CHECK(h5xx::read_attribute<int>(group, "level") == 2);
    BOOST_CHECK(boost::any_cast<int>(h5xx::read_attribute_if_exists<int>(group, "level")) == 2);

    group = h5xx::open_group(group, "three");
    BOOST_CHECK(group.getNumAttrs() == 1);
    BOOST_CHECK(h5xx::exists_attribute(group, "level"));
    BOOST_CHECK(h5xx::read_attribute<int>(group, "level") == 3);
    BOOST_CHECK(boost::any_cast<int>(h5xx::read_attribute_if_exists<int>(group, "level")) == 3);

    group = h5xx::open_group(group, "three");          // create new group
    BOOST_CHECK(group.getNumAttrs() == 0);
    BOOST_CHECK(!h5xx::exists_attribute(group, "level"));
    BOOST_CHECK_THROW(h5xx::read_attribute<int>(group, "level"), H5::AttributeIException);
    BOOST_CHECK(h5xx::read_attribute_if_exists<int>(group, "level").empty());

    // test h5xx::path
    BOOST_CHECK(h5xx::path(h5xx::open_group(file, "/")) == "/");
    BOOST_CHECK(h5xx::path(h5xx::open_group(file, "one/two/three")) == "/one/two/three");
    // note on previous check: semantics of h5xx::open_group seems to be somewhat too lazy

    file.close();

    // remove file
#ifdef NDEBUG
    unlink(filename);
#endif
}

// test h5xx::split_path separately
BOOST_AUTO_TEST_CASE( h5xx_split_path )
{
    std::vector<std::string> names(3);
    names[0] = "one";
    names[1] = "two";
    names[2] = "three";

    std::list<std::string> path = h5xx::split_path("/one/two/three/");
    BOOST_CHECK(path.size() == 3);
    BOOST_CHECK(std::equal(path.begin(), path.end(), names.begin()));

    path = h5xx::split_path("one/two/three");
    BOOST_CHECK(path.size() == 3);
    BOOST_CHECK(std::equal(path.begin(), path.end(), names.begin()));

    path = h5xx::split_path("//one///two//three");
    BOOST_CHECK(path.size() == 3);
    BOOST_CHECK(std::equal(path.begin(), path.end(), names.begin()));
}
