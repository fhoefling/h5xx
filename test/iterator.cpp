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

BOOST_AUTO_TEST_CASE( defaut_group )
{
    group container_group();
    group::dataset_iterator dset_iter;
    group::subgroup_iterator sgroup_iter;
    
    /** check iterator for invalid group TODO: should throw? **/
    
    BOOST_CHECK_THROW(dset_iter = container_group.begin());
    BOOST_CHECK_THROW(dset_iter = container_group.end());
    BOOST_CHECK_THROW(sgroup_iter = container_group.begin());
    BOOST_CHECK_THROW(sgroup_iter = container_group.end());
}


BOOST_AUTO_TEST_CASE( empty_group )
{
    group container_group(filename);
    
    // checking constructors TODO: default constructors?
    //BOOST_CHECK_NO_THROW(group::dataset_iterator());
    //BOOST_CHECK_NO_THROW(group::subgroup_iterator());
    BOOST_CHECK_NO_THROW(group::dataset_iterator(container_group));
    BOOST_CHECK_NO_THROW(group::subgroup_iterator(container_group));
    
    
    
    BOOST_CHECK_NO_THROW(group::dataset_iterator dset_iter_begin = container_group.begin());
    BOOST_CHECK_NO_THROW(group::dataset_iterator dset_iter_end = container_group.end());
    BOOST_CHECK_NO_THROW(group::subgroup_iterator sgroup_iter_begin = container_group.begin());
    BOOST_CHECK_NO_THROW(group::subgroup_iterator sgroup_iter_end = container_group.end());
    
    // begin- and end-iterator should be equal in empty group
    BOOST_CHECK_EQUAL(dset_iter_begin, dset_iter_end);
    BOOST_CHECK_EQUAL(sgroup_iter_begin, sgroup_iter_end);
    
    // check ==- and !=-operator
    BOOST_CHECK(dset_iter_begin == dset_iter_end);
    BOOST_CHECK(!(dset_iter_begin != dset_iter_end));
    BOOST_CHECK(sgroup_iter_begin == sgroup_iter_end);
    BOOST_CHECK(!(sgroup_iter_begin != sgroup_iter_end));
    
    // TODO: dereferencing in empty container?
    // BOOST_CHECK_THROW(); // in stl doesnt throw, but might give seg fault
}


BOOST_AUTO_TEST_CASE( only_datasets )
{
    group container_group(filename);
    dataset dset1 = create_dataset<int>(container_group, "test_dset1");
    dataset dset2 = create_dataset<int>(container_group, "test_dset2");
    dataset dset3 = create_dataset<int>(container_group, "test_dset3");
    dataset temp_set;
    
    BOOST_CHECK_NO_THROW(group::dataset_iterator dset_iter_begin = container_group.begin());
    BOOST_CHECK_NO_THROW(group::dataset_iterator dset_iter_end = container_group.end());
    BOOST_CHECK_NO_THROW(group::subgroup_iterator sgroup_iter_begin = container_group.begin());
    BOOST_CHECK_NO_THROW(group::subgroup_iterator sgroup_iter_end = container_group.end());
    
    // begin- and end-iterator over subgroups should be equal
    BOOST_CHECK_EQUAL(sgroup_iter_begin, sgroup_iter_end);
    
    // begin- and end-iterator over datasets should not be equal
    BOOST_CHECK(dset_iter_begin != dset_iter_end);
    
    // check operators
    temp_set = *dset_iter_begin++;
    BOOST_CHECK_EQUAL(temp_set.hid(), dset1.hid());
    BOOST_CHECK(dset_iter_begin != dset_iter_end);
    
    // check == and !=
    group::dataset_iterator dset_iterator_2 = container_group.begin();
    BOOST_CHECK(dset_iterator_begin != dset_iterator_2);
    dset_iterator++;
    BOOST_CHECK(dset_iterator_begin == dset_iterator_2);
    
    temp_set = *dset_iter_begin;
    BOOST_CHECK_EQUAL(temp_set.hid(), dset2.hid());
    BOOST_CHECK(dset_iter_begin != dset_iter_end);
    
    temp_set = *++dset_iter_begin;
    BOOST_CHECK_EQUAL(temp_set.hid(), dset3.hid());
    BOOST_CHECK(dset_iter_begin != dset_iter_end);
    
    temp_set = *dset_iter_begin;
    BOOST_CHECK_EQUAL(temp_set.hid(), dset3.hid());
    BOOST_CHECK(dset_iter_begin != dset_iter_end);
    
    dset_iter_begin++;
    BOOST_CHECK_EQUAL(dset_iter_begin, dset_iter_end); // TODO: is this equal to BOOST_CHECK(dset_iter_begin == dset_iter_end) ?
}


BOOST_AUTO_TEST_CASE( only_subgroups )
{
    group container_group(filename);
    group grp1(container_group, "test_grp1");
    group grp2(container_group, "test_grp2");
    group grp3(container_group, "test_grp3");
    group temp_grp;
    
    BOOST_CHECK_NO_THROW(group::dataset_iterator dset_iter_begin = container_group.begin());
    BOOST_CHECK_NO_THROW(group::dataset_iterator dset_iter_end = container_group.end());
    BOOST_CHECK_NO_THROW(group::subgroup_iterator sgroup_iter_begin = container_group.begin());
    BOOST_CHECK_NO_THROW(group::subgroup_iterator sgroup_iter_end = container_group.end());
    
    // begin- and end-iterator over subgroups should not be equal
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_end);
    
    // begin- and end-iterator over datasets should be equal
    BOOST_CHECK_EQUAL(dset_iter_begin, dset_iter_end);
    
    // check operators
    temp_grp = *sgroup_iter_begin++;
    BOOST_CHECK_EQUAL(temp_grp.hid(), grp1.hid());
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_end);
    
    // check == and !=
    group::subgroup_iterator sgroup_iter_2 = container_group.begin();
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_2);
    sgroup_iter_2++;
    BOOST_CHECK(sgroup_iter_begin == sgroup_iter_2);
    
    temp_grp = *sgroup_iter_begin;
    BOOST_CHECK_EQUAL(temp_grp.hid(), grp2.hid());
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_end);
    
    temp_grp = *++sgroup_iter_begin;
    BOOST_CHECK_EQUAL(temp_grp.hid(), grp3.hid());
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_end);
    
    temp_grp = *sgroup_iter_begin;
    BOOST_CHECK_EQUAL(temp_grp.hid(), grp3.hid());
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_end);
    
    sgroup_iter_begin++;
    BOOST_CHECK_EQUAL(sgroup_iter_begin, sgroup_iter_end); // TODO: see above
}


BOOST_AUTO_TEST_CASE( mixed_1 )
{
    /** container group with 2 datasets and 1 subgroup **/
    group container_group(filename);
    group grp1(container_group, "test_grp1");
    dataset dset1(container_group, "test_dset1");
    dataset dset2(container_group, "test_dset2");
    group temp_grp;
    dataset temp_set;
        
    BOOST_CHECK_NO_THROW(group::dataset_iterator dset_iter_begin = container_group.begin());
    BOOST_CHECK_NO_THROW(group::dataset_iterator dset_iter_end = container_group.end());
    BOOST_CHECK_NO_THROW(group::subgroup_iterator sgroup_iter_begin = container_group.begin());
    BOOST_CHECK_NO_THROW(group::subgroup_iterator sgroup_iter_end = container_group.end());
    
    // begin- and end-iterator over subgroups should not be equal
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_end);
    
    // begin- and end-iterator over datasets should not be equal
    BOOST_CHECK(dset_iter_begin != dset_iter_end);
    
    // check operators
    // iterate subgroup iter
    temp_grp = *sgroup_iter_begin++;
    BOOST_CHECK_EQUAL(temp_grp.hid(), grp1.hid());
    
    BOOST_CHECK_EQUAL(sgroup_iter_begin, sgroup_iter_end);
    
    // iterate dataset iter
    temp_set = *dset_iter_begin++;
    BOOST_CHECK_EQUAL(temp_set.hid(), dset1.hid());
    BOOST_CHECK(dset_iter_begin != dset_iter_end);
    
    temp_set = *dset_iter_begin++;
    BOOST_CHECK_EQUAL(temp_set.hid(), dset2);
    
    BOOST_CHECK_EQUAL(dset_iter_begin, dset_iter_end); 
}


BOOST_AUTO_TEST_CASE( mixed_2 )
{
    /** container group with 1 dataset and 2 subgroups **/
    group container_group(filename);
    group grp1(container_group, "test_grp1");
    group grp2(container_group, "test_grp2");
    dataset dset1(container_group, "test_dset1");
    group temp_grp;
    dataset temp_set;
    
    
    BOOST_CHECK_NO_THROW(group::dataset_iterator dset_iter_begin = container_group.begin());
    BOOST_CHECK_NO_THROW(group::dataset_iterator dset_iter_end = container_group.end());
    BOOST_CHECK_NO_THROW(group::subgroup_iterator sgroup_iter_begin = container_group.begin());
    BOOST_CHECK_NO_THROW(group::subgroup_iterator sgroup_iter_end = container_group.end());
    
    // begin- and end-iterator over subgroups should not be equal
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_end);
    
    // begin- and end-iterator over datasets should not be equal
    BOOST_CHECK(dset_iter_begin != dset_iter_end);
    
    // check operators
    // iterate subgroup iter
    temp_grp = *sgroup_iter_begin++;
    BOOST_CHECK_EQUAL(temp_grp.hid(), grp1.hid());
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_end);
    
    temp_grp = *sgroup_iter_begin++;
    BOOST_CHECK_EQUAL(temp_grp.hid(), grp2.hid());
    
    BOOST_CHECK_EQUAL(sgroup_iter_begin, sgroup_iter_end);
    
    // iterate dataset iter
    temp_set = *dset_iter_begin++;
    BOOST_CHECK_EQUAL(temp_set.hid(), dset1.hid());
    
    BOOST_CHECK_EQUAL(dset_iter_begin, dset_iter_end);
}


BOOST_AUTO_TEST_CASE( mixed_3 )
{
    /** container group with 2 datasets and 2 subgroups **/
    group container_group(filename);
    group grp1(container_group, "test_grp1");
    group grp2(container_group, "test_grp2");
    dataset dset1(container_group, "test_dset1");
    dataset dset2(container_group, "test_dset2");
    group temp_grp;
    dataset temp_set;
       
    BOOST_CHECK_NO_THROW(group::dataset_iterator dset_iter_begin = container_group.begin());
    BOOST_CHECK_NO_THROW(group::dataset_iterator dset_iter_end = container_group.end());
    BOOST_CHECK_NO_THROW(group::subgroup_iterator sgroup_iter_begin = container_group.begin());
    BOOST_CHECK_NO_THROW(group::subgroup_iterator sgroup_iter_end = container_group.end());
    
    // begin- and end-iterator over subgroups should not be equal
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_end);
    
    // begin- and end-iterator over datasets should not be equal
    BOOST_CHECK(dset_iter_begin != dset_iter_end);
    
    // check == and !=
    BOOST_CHECK_THROW(dset_iter_begin == sgroup_iter_begin);
    BOOST_CHECK_THROW(dset_iter_begin != sgroup_iter_begin);
    
    // check operators
    // iterate  subgroup iter
    temp_grp = *sgroup_iter_begin++;
    BOOST_CHECK_EQUAL(temp_grp.hid(), grp1.hid());
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_end);
    
    temp_grp = *sgroup_iter_begin++;
    BOOST_CHECK_EQUAL(temp_grp.hid(), grp2.hid());
    
    BOOST_CHECK_EQUAL(sgroup_iter_begin, sgroup_iter_end);
    
    // iterate dataset iter
    temp_set = *dset_iter_begin++;
    BOOST_CHECK_EQUAL(temp_set.hid(), dset1.hid());
    BOOST_CHECK(dset_iter_begin != dset_iter_end);
    
    temp_set = *dset_iter_begin++;
    BOOST_CHECK_EQUAL(temp_set.hid(), dset2);
    
    BOOST_CHECK_EQUAL(dset_iter_begin, dset_iter_end);
}
} // namespace fixture
