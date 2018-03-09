/*
 * Copyright © 2018 Matthias Werner and Felix Höfling
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
#include <h5xx/dataset.hpp>
#include <h5xx/iterator.hpp>

#include <test/ctest_full_output.hpp>
#include <test/catch_boost_no_throw.hpp>
#include <test/fixture.hpp>


namespace fixture
{

char filename[] = "test_h5xx_group.h5";
typedef h5file<filename> BOOST_AUTO_TEST_CASE_FIXTURE;

using namespace h5xx;

BOOST_AUTO_TEST_CASE( def group )
{
    group container_group();
}


BOOST_AUTO_TEST_CASE( empty group )
{
    group container_group(filename);
}


BOOST_AUTO_TEST_CASE( only datasets )
{
    group container_group(filename);
    dataset dset1 = create_dataset<int>(container_group, "test_dset1");
    dataset dset2 = create_dataset<int>(container_group, "test_dset2");
    dataset dset3 = create_dataset<int>(container_group, "test_dset3");
    
    
}


BOOST_AUTO_TEST_CASE( only subgroups )
{
    group container_group(filename);
    group grp1(container_group, "test_grp1");
    group grp2(container_group, "test_grp2");
    group grp3(container_group, "test_grp3");
    
    
}


BOOST_AUTO_TEST_CASE( mixed 1 )
{
    group container_group(filename);
}


BOOST_AUTO_TEST_CASE( mixed 2 )
{
    group container_group(filename);
}
} // namespace fixture
