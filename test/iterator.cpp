/*
 * Copyright © 2018 Matthias Werner and Felix Höfling
 * All rights reserved.
 *
 * This file is part of h5xx — a C++ wrapper for the HDF5 library.
 *
 * This software may be modified and distributed under the terms of the
 * 3-clause BSD license.  See accompanying file LICENSE for details.
 */


/** Requirements fo Iterator(It) (according to http://en.cppreference.com/w/cpp/concept/ForwardIterator)
 *
 *  - DefaultConstructible
 *  - CopyConstructible
 *  - CopyAssignable
 *  - EqualityComparable
 *  - Destructible
 *  - lvalues are Swappable
 *  - std::iterator_traits<It> has member typedefs value_type, difference_type, reference, pointer and iterator_category
 *
 *  and
 *
 *  - operator* is defined
 *  - operator++() is defined (returns reference to iterator)
 *  - operator++(int)
 *  - (void) i++ equivalent (void)++i //FIXME what is this supposed to mean?
 *  - operator!= / operator==
 *  - It->m should be equivalent to (*It).m
 *
 *  and (for ForwarIterator)
 *
 *  - Multipass guarantee
 *
 **/

#define BOOST_TEST_MODULE h5xx_group
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <h5xx/group.hpp>
#include <h5xx/dataset.hpp>

#include <test/ctest_full_output.hpp>
#include <test/catch_boost_no_throw.hpp>
#include <test/fixture.hpp>

namespace fixture
{

char filename[] = "test_h5xx_iterator.h5";
typedef h5file<filename> BOOST_AUTO_TEST_CASE_FIXTURE;

using namespace h5xx;

// FIXME test begin(), end() in test/group.cpp,
// for default constructed group, empty group, group with 1 and 2 members.

BOOST_AUTO_TEST_CASE( default_group )
{
    BOOST_TEST_MESSAGE("Testing iterator over default constructed group");
    group container_group();
/**
    BOOST_CHECK_NO_THROW(dataset_container::iterator dset_iter_begin = container_group.datasets().begin());
    BOOST_CHECK_NO_THROW(subgroup_container::iterator sgroup_iter_begin = container_group.subgroups().begin());
    BOOST_CHECK_NO_THROW(dataset_container::iterator dset_iter_end = container_group.datasets().end());
    BOOST_CHECK_NO_THROW(subgroup_container::iterator sgroup_iter_end = container_group.subgroups().end());

    dataset_container::iterator dset_iter_begin = container_group.datasets().begin();
    dataset_container::iterator dset_iter_end = container_group.datasets().end();
    subgroup_container::iterator sgroup_iter_begin = container_group.subgroups().begin();
    subgroup_container::iterator sgroup_iter_end = container_group.subgroups().end();

    BOOST_CHECK(dset_iter_begin == dset_iter_end);
    BOOST_CHECK(sgroup_iter_begin == sgroup_iter_begin);
**/
}


BOOST_AUTO_TEST_CASE( iterator_requirements )
{
    BOOST_TEST_MESSAGE("Testing basic requirements of ForwardIterator");
    
    dataset_container::iterator dset_iter, dset_iter_2, dset_iter_3;
    subgroup_container::iterator sgroup_iter, sgroup_iter_2, sgroup_iter_3;

    /** check DefaultConstructible and MoveAssignable*/
    BOOST_CHECK_NO_THROW(dset_iter = dataset_container::iterator());
    BOOST_CHECK_NO_THROW(sgroup_iter = subgroup_container::iterator());

    /** check MoveConstructible */
    BOOST_CHECK_NO_THROW(dataset_container::iterator temp = dataset_container::iterator());
    BOOST_CHECK_NO_THROW(dataset_container::iterator temp(dataset_container::iterator()));
    BOOST_CHECK_NO_THROW(subgroup_container::iterator temp = subgroup_container::iterator());
    BOOST_CHECK_NO_THROW(subgroup_container::iterator temp(subgroup_container::iterator()));

    /** check CopyConstructible */
    BOOST_CHECK_NO_THROW(dset_iter_2 = dataset_container::iterator(dset_iter));
    BOOST_CHECK_NO_THROW(sgroup_iter_2 = subgroup_container::iterator(sgroup_iter));

    /** check CopyAssignable */
    BOOST_CHECK_NO_THROW(dset_iter_3 = dset_iter;);
    BOOST_CHECK_NO_THROW(sgroup_iter_3 = sgroup_iter);

    /** check EqualityComparable */
    BOOST_CHECK(dset_iter == dset_iter_2);
    BOOST_CHECK(sgroup_iter == sgroup_iter_2);

    /** check lvalues are Swappable
     *  TODO: is this the same as MoveConstructible && MoveAssignable? */
    BOOST_CHECK_NO_THROW(std::swap(dset_iter, dset_iter_2));
    BOOST_CHECK_NO_THROW(std::swap(sgroup_iter, sgroup_iter_2));

/*  TODO: std::iterator_traits<It> has member typedefs value_type, difference_type, reference, pointer and iterator_category
 */

}

BOOST_AUTO_TEST_CASE( iterator_expressions )
{
    BOOST_TEST_MESSAGE("Testing basic iterator expressions");

    dataset_container::iterator dset_iter;
    dataset_container::iterator dset_iter_2;
    subgroup_container::iterator sgroup_iter;
    subgroup_container::iterator sgroup_iter_2;

    /** check operator* 
     *  should throw h5xx::error, since container_group_ does not exist */
    BOOST_CHECK_THROW(*sgroup_iter, std::invalid_argument);
    BOOST_CHECK_THROW(*dset_iter, std::invalid_argument);

    /** check It->m equivalent to (*It).m */
    /** FIXME: How to check for this?
    BOOST_CHECK(dset_iter-> == (*dset_iter). );
    BOOST_CHECK(sgroup_iter-> == (*sgroup_iter). );
     **/

    /** check operator++() */
    BOOST_CHECK_THROW(++dset_iter, std::invalid_argument);
    BOOST_CHECK_THROW(++sgroup_iter, std::invalid_argument);

    /** check operator++(int) */
    BOOST_CHECK_THROW(dset_iter++, std::invalid_argument);
    BOOST_CHECK_THROW(sgroup_iter++, std::invalid_argument);
    
    /** TODO: check (void) i++ == (void)++i */
     
    
    /** check operator!= / operator== */ 
    BOOST_CHECK(dset_iter == dset_iter_2);
    BOOST_CHECK(!(dset_iter != dset_iter_2));
    BOOST_CHECK(sgroup_iter == sgroup_iter_2);
    BOOST_CHECK(!(sgroup_iter != sgroup_iter_2));
    
    /** TODO: check Multipass guarantee */

}

BOOST_AUTO_TEST_CASE( empty_group )
{
    group container_group(file);
    // check default constructors
    BOOST_CHECK_NO_THROW(subgroup_container::iterator());
    BOOST_CHECK_NO_THROW(dataset_container::iterator());

    // check constructors from file
    BOOST_CHECK_NO_THROW(subgroup_container::iterator(container_group));
    BOOST_CHECK_NO_THROW(dataset_container::iterator(container_group));

    BOOST_CHECK_NO_THROW(dataset_container::iterator dset_iter_begin = container_group.datasets().begin());
    BOOST_CHECK_NO_THROW(dataset_container::iterator dset_iter_end = container_group.datasets().end());
    BOOST_CHECK_NO_THROW(subgroup_container::iterator sgroup_iter_begin = container_group.subgroups().begin());
    BOOST_CHECK_NO_THROW(subgroup_container::iterator sgroup_iter_end = container_group.subgroups().end());


    dataset_container::iterator dset_iter_begin = container_group.datasets().begin();
    dataset_container::iterator dset_iter_end = container_group.datasets().end();
    subgroup_container::iterator sgroup_iter_begin = container_group.subgroups().begin();
    subgroup_container::iterator sgroup_iter_end = container_group.subgroups().end();

    // check CopyConstructor
    BOOST_CHECK_NO_THROW(dataset_container::iterator dset_iter_begin_2(dset_iter_begin));
    BOOST_CHECK_NO_THROW(subgroup_container::iterator sgroup_iter_begin_2(sgroup_iter_begin));

    // check CopyAssignable
    dataset_container::iterator dset_iter_begin_2;
    subgroup_container::iterator sgroup_iter_begin_2;

    BOOST_CHECK_NO_THROW(dset_iter_begin_2 = dset_iter_begin);
    BOOST_CHECK_NO_THROW(sgroup_iter_begin_2 = sgroup_iter_begin);

    // begin- and end-iterator should be equal in empty group
    BOOST_CHECK(dset_iter_begin == dset_iter_end);
    BOOST_CHECK(sgroup_iter_begin == sgroup_iter_end);

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
    group container_group(file);

    dataset dset1 = create_dataset<int>(container_group, "dset1");
    dataset dset2 = create_dataset<int>(container_group, "dset2");
    dataset dset3 = create_dataset<int>(container_group, "dset3");

    // testing begin(), end()
    dataset_container::iterator dset_iter_begin = container_group.datasets().begin();
    dataset_container::iterator dset_iter_end = container_group.datasets().end();
    dataset_container::iterator dset_iter_begin_2 = container_group.datasets().begin();

    subgroup_container::iterator sgroup_iter_begin = container_group.subgroups().begin();
    subgroup_container::iterator sgroup_iter_end = container_group.subgroups().end();

    // no subgroups: begin and end iterators over subgroups should be equal
    BOOST_CHECK(sgroup_iter_begin == sgroup_iter_end);

    // begin and end iterators over datasets should not be equal
    BOOST_CHECK(dset_iter_begin != dset_iter_end);
    BOOST_CHECK(dset_iter_begin == dset_iter_begin_2++);    // test post-increment

// FIXME    BOOST_CHECK_EQUAL(dset_iter_begin.name() == "dset1");

    // check operators / expressions FIXME to seperate test
    BOOST_CHECK((*dset_iter_begin).valid());

    ++dset_iter_begin;
    BOOST_CHECK((*dset_iter_begin).valid());
    BOOST_CHECK(dset_iter_begin != dset_iter_end);

    ++dset_iter_begin;
    BOOST_CHECK((*dset_iter_begin).valid());
    BOOST_CHECK(dset_iter_begin != dset_iter_end);

    ++dset_iter_begin;
    BOOST_CHECK(dset_iter_begin == dset_iter_end);

    BOOST_CHECK(dset_iter_begin != dset_iter_begin_2);
    ++dset_iter_begin_2;
    ++dset_iter_begin_2;
    BOOST_CHECK(dset_iter_begin_2 == dset_iter_end);
}


BOOST_AUTO_TEST_CASE( only_subgroups )
{
    group container_group(file);
    group grp1(container_group, "test_grp1");
    group grp2(container_group, "test_grp2");
    group grp3(container_group, "test_grp3");

    BOOST_CHECK_NO_THROW(dataset_container::iterator dset_iter_begin = container_group.datasets().begin());
    BOOST_CHECK_NO_THROW(dataset_container::iterator dset_iter_end = container_group.datasets().end());
    BOOST_CHECK_NO_THROW(subgroup_container::iterator sgroup_iter_begin = container_group.subgroups().begin());
    BOOST_CHECK_NO_THROW(subgroup_container::iterator sgroup_iter_end = container_group.subgroups().end());

    dataset_container::iterator dset_iter_begin = container_group.datasets().begin();
    dataset_container::iterator dset_iter_end = container_group.datasets().end();

    subgroup_container::iterator sgroup_iter_begin = container_group.subgroups().begin();
    subgroup_container::iterator sgroup_iter_end = container_group.subgroups().end();
    subgroup_container::iterator sgroup_iter_begin_2 = container_group.subgroups().begin();

    // begin- and end-iterator over subgroups should not be equal
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_end);
    BOOST_CHECK(sgroup_iter_begin == sgroup_iter_begin_2++);

    // begin- and end-iterator over datasets should be equal
    BOOST_CHECK(dset_iter_begin == dset_iter_end);

    // check operators
    BOOST_CHECK((*sgroup_iter_begin).valid());

    ++sgroup_iter_begin;
    BOOST_CHECK((*sgroup_iter_begin).valid());
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_end);

    ++sgroup_iter_begin;
    BOOST_CHECK((*sgroup_iter_begin).valid());
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_end);

    ++sgroup_iter_begin;
    BOOST_CHECK(sgroup_iter_begin == sgroup_iter_end);

    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_begin_2);
    ++sgroup_iter_begin_2;
    ++sgroup_iter_begin_2;
    BOOST_CHECK(sgroup_iter_begin_2 == sgroup_iter_end);
}


BOOST_AUTO_TEST_CASE( mixed_1 )
{
    /** container group with 2 datasets and 1 subgroup **/
    group container_group(file);
    group grp1(container_group, "test_grp1");
    dataset dset1 = create_dataset<int>(container_group, "test_dset1");
    dataset dset2 = create_dataset<int>(container_group, "test_dset2");
    
    BOOST_CHECK_NO_THROW(dataset_container::iterator dset_iter_begin = container_group.datasets().begin());
    BOOST_CHECK_NO_THROW(dataset_container::iterator dset_iter_end = container_group.datasets().end());
    BOOST_CHECK_NO_THROW(subgroup_container::iterator sgroup_iter_begin = container_group.subgroups().begin());
    BOOST_CHECK_NO_THROW(subgroup_container::iterator sgroup_iter_end = container_group.subgroups().end());

    dataset_container::iterator dset_iter_begin = container_group.datasets().begin();
    dataset_container::iterator dset_iter_end = container_group.datasets().end();
    subgroup_container::iterator sgroup_iter_begin = container_group.subgroups().begin();
    subgroup_container::iterator sgroup_iter_end = container_group.subgroups().end();

    // begin- and end-iterator over subgroups should not be equal
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_end);

    // begin- and end-iterator over datasets should not be equal
    BOOST_CHECK(dset_iter_begin != dset_iter_end);

    // check operators
    // iterate subgroup iter
    BOOST_CHECK((*sgroup_iter_begin++).valid());
    BOOST_CHECK(sgroup_iter_begin == sgroup_iter_end);
    
    // iterate dataset iter
    BOOST_CHECK((*dset_iter_begin++).valid());
    BOOST_CHECK(dset_iter_begin != dset_iter_end);
}


BOOST_AUTO_TEST_CASE( mixed_2 )
{
    /** container group with 1 dataset and 2 subgroups **/
    group container_group(file);
    group grp1(container_group, "test_grp1");
    group grp2(container_group, "test_grp2");
    dataset dset1 = create_dataset<int>(container_group, "test_dset1");
    dataset temp_set;
    group temp_group;

    BOOST_CHECK_NO_THROW(dataset_container::iterator dset_iter_begin = container_group.datasets().begin());
    BOOST_CHECK_NO_THROW(dataset_container::iterator dset_iter_end = container_group.datasets().end());
    BOOST_CHECK_NO_THROW(subgroup_container::iterator sgroup_iter_begin = container_group.subgroups().begin());
    BOOST_CHECK_NO_THROW(subgroup_container::iterator sgroup_iter_end = container_group.subgroups().end());

    dataset_container::iterator dset_iter_begin = container_group.datasets().begin();
    dataset_container::iterator dset_iter_end = container_group.datasets().end();
    subgroup_container::iterator sgroup_iter_begin = container_group.subgroups().begin();
    subgroup_container::iterator sgroup_iter_end = container_group.subgroups().end();


    // begin- and end-iterator over subgroups should not be equal
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_end);

    // begin- and end-iterator over datasets should not be equal
    BOOST_CHECK(dset_iter_begin != dset_iter_end);

    // check operators
    // iterate subgroup iter
    BOOST_CHECK((*sgroup_iter_begin++).valid());
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_end);
    
    // iterate dataset iter
    //BOOST_CHECK((*dset_iter_begin++).valid());
    ++dset_iter_begin;
    BOOST_CHECK(dset_iter_begin == dset_iter_end);
}


BOOST_AUTO_TEST_CASE( mixed_3 )
{
    /** container group with 2 datasets and 2 subgroups **/
    group container_group(file);
    group grp1(container_group, "test_grp1");
    group grp2(container_group, "test_grp2");
    dataset dset1 = create_dataset<int>(container_group, "test_dset1");
    dataset dset2 = create_dataset<int>(container_group, "test_dset2");

    BOOST_CHECK_NO_THROW(dataset_container::iterator dset_iter_begin = container_group.datasets().begin());
    BOOST_CHECK_NO_THROW(dataset_container::iterator dset_iter_end = container_group.datasets().end());
    BOOST_CHECK_NO_THROW(subgroup_container::iterator sgroup_iter_begin = container_group.subgroups().begin());
    BOOST_CHECK_NO_THROW(subgroup_container::iterator sgroup_iter_end = container_group.subgroups().end());

    dataset_container::iterator dset_iter_begin = container_group.datasets().begin();
    dataset_container::iterator dset_iter_end = container_group.datasets().end();
    subgroup_container::iterator sgroup_iter_begin = container_group.subgroups().begin();
    subgroup_container::iterator sgroup_iter_end = container_group.subgroups().end();

    // begin- and end-iterator over subgroups should not be equal
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_end);

    // begin- and end-iterator over datasets should not be equal
    BOOST_CHECK(dset_iter_begin != dset_iter_end);
    
    // check operators
    // iterate subgroup iter
    BOOST_CHECK((*sgroup_iter_begin++).valid());
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_end);
    
    // iterate dataset iter
    BOOST_CHECK((*dset_iter_begin++).valid());
    BOOST_CHECK(dset_iter_begin != dset_iter_end);
}
} // namespace fixture
