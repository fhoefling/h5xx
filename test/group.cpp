/*
 * Copyright © 2010-2013 Felix Höfling
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

#include <h5xx/attribute.hpp>
#include <h5xx/group.hpp>
#include <h5xx/utility.hpp>

#include<istream>

#include <test/ctest_full_output.hpp>
#include <test/catch_boost_no_throw.hpp>
#include <test/fixture.hpp>

BOOST_GLOBAL_FIXTURE( ctest_full_output )

char filename[] = "test_h5xx_group.h5";
BOOST_FIXTURE_TEST_SUITE( dummy, h5file<filename> )

using namespace h5xx;

BOOST_AUTO_TEST_CASE( h5xx_group_remake )
{
    char const* foogroup  = "foo";
    group h5group;
    BOOST_CHECK_NO_THROW(h5group.open(file, foogroup));
    BOOST_CHECK(get_name(h5group) == "foogroup");

    group one, two, three, four;
    BOOST_CHECK_NO_THROW(one.open(file, "one"));
    BOOST_CHECK_NO_THROW(three.open(one, "two/three"));
    BOOST_CHECK(exists_group(one, "two"));
    BOOST_CHECK_NO_THROW(two.open(one, "two"));
    BOOST_CHECK_NO_THROW(four.open(two, "four"));
    BOOST_CHECK(get_name(one) == "/one");
    BOOST_CHECK(get_name(two) == "/one/two");
    BOOST_CHECK(get_name(three) == "/one/two/three");
    BOOST_CHECK(get_name(four) == "/one/two/four");

    BOOST_CHECK_NO_THROW(h5group.close());
}

BOOST_AUTO_TEST_CASE( h5xx_group )
{
    file.close(); H5::H5File file(::filename, H5F_ACC_TRUNC); // FIXME remove this line

    BOOST_CHECK_NO_THROW(h5xx::open_group(file, "/"));
    BOOST_CHECK_THROW(h5xx::open_group(file, ""), h5xx::error);

    H5::Group group = h5xx::open_group(file, "/");
    BOOST_CHECK_NO_THROW(h5xx::open_group(group, "/"));
    BOOST_CHECK_THROW(h5xx::open_group(group, ""), h5xx::error);

    // create a hierarchy of groups
    h5xx::open_group(file, "/");
    h5xx::open_group(file, "/one");
    h5xx::open_group(file, "/one/two");
    h5xx::open_group(file, "/one/two/three");

    group = h5xx::open_group(file, "one");
//     BOOST_CHECK(group.getNumAttrs() == 1);
//     BOOST_CHECK(h5xx::exists_attribute(group, "level"));
//     BOOST_CHECK(h5xx::read_attribute<int>(group, "level") == 1);
//     BOOST_CHECK(boost::any_cast<int>(h5xx::read_attribute_if_exists<int>(group, "level")) == 1);

    h5xx::open_group(group, "branch");        // create branch in '/one'
//     BOOST_CHECK(group.getNumAttrs() == 1);
    BOOST_CHECK(group.getNumObjs() == 2);

    group = h5xx::open_group(group, "two/three/");
//     BOOST_CHECK(group.getNumAttrs() == 1);
//     BOOST_CHECK(h5xx::exists_attribute(group, "level"));
//     BOOST_CHECK(h5xx::read_attribute<int>(group, "level") == 3);
//     BOOST_CHECK(boost::any_cast<int>(h5xx::read_attribute_if_exists<int>(group, "level")) == 3);

    group = h5xx::open_group(file, "one/two");
//     BOOST_CHECK(group.getNumAttrs() == 1);
//     BOOST_CHECK(h5xx::exists_attribute(group, "level"));
//     BOOST_CHECK(h5xx::read_attribute<int>(group, "level") == 2);
//     BOOST_CHECK(boost::any_cast<int>(h5xx::read_attribute_if_exists<int>(group, "level")) == 2);

    group = h5xx::open_group(group, "three");
//     BOOST_CHECK(group.getNumAttrs() == 1);
//     BOOST_CHECK(h5xx::exists_attribute(group, "level"));
//     BOOST_CHECK(h5xx::read_attribute<int>(group, "level") == 3);
//     BOOST_CHECK(boost::any_cast<int>(h5xx::read_attribute_if_exists<int>(group, "level")) == 3);

    group = h5xx::open_group(group, "three");          // create new group
//     BOOST_CHECK(group.getNumAttrs() == 0);
//     BOOST_CHECK(!h5xx::exists_attribute(group, "level"));
//     H5E_BEGIN_TRY{
//         BOOST_CHECK_THROW(h5xx::read_attribute<int>(group, "level"), H5::AttributeIException);
//     }H5E_END_TRY
//     BOOST_CHECK(h5xx::read_attribute_if_exists<int>(group, "level").empty());

    // test h5xx::path
    BOOST_CHECK(h5xx::path(h5xx::open_group(file, "/")) == "/");
    BOOST_CHECK(h5xx::path(h5xx::open_group(file, "one/two/three")) == "/one/two/three");
    // note on previous check: semantics of h5xx::open_group seems to be somewhat too lazy
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

BOOST_AUTO_TEST_SUITE_END()
