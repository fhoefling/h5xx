/*
 * Copyright © 2018 Matthias Werner
 * Copyright © 2018 Felix Höfling
 * All rights reserved.
 *
 * This file is part of h5xx — a C++ wrapper for the HDF5 library.
 *
 * This software may be modified and distributed under the terms of the
 * 3-clause BSD license.  See accompanying file LICENSE for details.
 */

/**
 * Requirements fo Iterator(It)
 * http://en.cppreference.com/w/cpp/concept/ForwardIterator
 *
 *  - DefaultConstructible
 *  - CopyConstructible
 *  - CopyAssignable
 *  - EqualityComparable
 *  - Destructible
 *  - lvalues are Swappable
 *  - std::iterator_traits<It> has member typedefs:
 *    value_type, difference_type, reference, pointer and iterator_category
 *
 *  and
 *
 *  - operator* is defined - tested
 *  - operator++() is defined (returns reference to iterator) - tested
 *  - operator++(int) - tested
 *  - (void) i++ equivalent (void)++i - tested
 *  - operator!= / operator== - tested
 *  - It->m should be equivalent to (*It).m -tested
 *
 *  and (for ForwarIterator)
 *
 *  - Multipass guarantee - tested
 *
 */
#define BOOST_TEST_MODULE h5xx_group
#include <boost/test/unit_test.hpp>

// #include <h5xx/dataset.hpp>  FIXME dataset.hpp must be included after group.hpp
#include <h5xx/group.hpp>
#include <h5xx/dataset.hpp>

#include <test/ctest_full_output.hpp>
#include <test/catch_boost_no_throw.hpp>
#include <test/fixture.hpp>

BOOST_GLOBAL_FIXTURE( ctest_full_output );

namespace fixture { // preferred over BOOST_FIXTURE_TEST_SUITE

char filename[] = "test_h5xx_iterator.h5";
typedef h5file<filename> BOOST_AUTO_TEST_CASE_FIXTURE;

using namespace h5xx;

// FIXME test begin(), end() in test/group.cpp,
// for default constructed group, empty group, group with 1 and 2 members.

BOOST_AUTO_TEST_CASE( iterator_constructors )
{
    BOOST_TEST_MESSAGE("\nTesting iterator constructors");

    group container_group(file);

    /** check default constructor */
    BOOST_TEST_MESSAGE("testing default constructor");
    BOOST_CHECK_NO_THROW(container<dataset>::iterator dset_iter);
    BOOST_CHECK_NO_THROW(container<group>::iterator sgroup_iter);

    container<dataset>::iterator dset_iter;
    container<group>::iterator sgroup_iter;

    BOOST_CHECK_NO_THROW(dset_iter = container<dataset>::iterator());  // TODO why the assignments here?
    BOOST_CHECK_NO_THROW(sgroup_iter = container<group>::iterator());

    /** check non-default constructor */
    BOOST_TEST_MESSAGE("testing constructor on (empty) group");
    BOOST_CHECK_NO_THROW(dset_iter = container<dataset>::iterator(container_group));
    BOOST_CHECK_NO_THROW(sgroup_iter = container<group>::iterator(container_group));

    /** check copy constructor */
    BOOST_TEST_MESSAGE("testing copy constructor");

    BOOST_CHECK_NO_THROW(container<dataset>::iterator dset_iter_2 = dset_iter);
    BOOST_CHECK_NO_THROW(container<group>::iterator sgroup_iter_2 = sgroup_iter);
}

BOOST_AUTO_TEST_CASE( iterator_requirements )
{
    BOOST_TEST_MESSAGE("\nTesting basic requirements of ForwardIterator");

    container<dataset>::iterator dset_iter, dset_iter_2, dset_iter_3;
    container<group>::iterator sgroup_iter, sgroup_iter_2, sgroup_iter_3;

    /** check DefaultConstructible and MoveAssignable*/
    BOOST_TEST_MESSAGE("testing DefaultConstructible and MoveAssignable");
    BOOST_CHECK_NO_THROW(dset_iter = container<dataset>::iterator());
    BOOST_CHECK_NO_THROW(sgroup_iter = container<group>::iterator());

    /** check MoveConstructible */
    BOOST_TEST_MESSAGE("testing MoveConstructible");
    BOOST_CHECK_NO_THROW(container<dataset>::iterator temp = container<dataset>::iterator());
    BOOST_CHECK_NO_THROW(container<dataset>::iterator temp(container<dataset>::iterator()));
    BOOST_CHECK_NO_THROW(container<group>::iterator temp = container<group>::iterator());
    BOOST_CHECK_NO_THROW(container<group>::iterator temp(container<group>::iterator()));

    /** check CopyConstructible */
    BOOST_TEST_MESSAGE("testing CopyConstructible");
    BOOST_CHECK_NO_THROW(dset_iter_2 = container<dataset>::iterator(dset_iter));
    BOOST_CHECK_NO_THROW(sgroup_iter_2 = container<group>::iterator(sgroup_iter));

    /** check CopyAssignable */
    BOOST_TEST_MESSAGE("testing CopyAssignable");
    BOOST_CHECK_NO_THROW(dset_iter_3 = dset_iter;);
    BOOST_CHECK_NO_THROW(sgroup_iter_3 = sgroup_iter);
    BOOST_CHECK(dset_iter_3 == dset_iter);
    BOOST_CHECK(sgroup_iter_3 == sgroup_iter);

    /** check EqualityComparable */
    BOOST_TEST_MESSAGE("testing EqualityComparable");
    BOOST_CHECK(dset_iter == dset_iter_2);
    BOOST_CHECK(sgroup_iter == sgroup_iter_2);

    /** check lvalues are Swappable */
    BOOST_TEST_MESSAGE("testing lvalues are swappable");
    BOOST_CHECK_NO_THROW(std::swap(dset_iter, dset_iter_2));
    BOOST_CHECK_NO_THROW(std::swap(sgroup_iter, sgroup_iter_2));
 
    /** check member typedefs value_type, difference_type, reference, pointer and iterator_category */
    BOOST_TEST_MESSAGE("testing std::iterator_traits");
    BOOST_CHECK(typeid(dataset_container::iterator::value_type) == typeid(dataset));
    BOOST_CHECK(typeid(subgroup_container::iterator::value_type) == typeid(group));
    
    BOOST_CHECK(typeid(dataset_container::iterator::difference_type) == typeid(std::ptrdiff_t));
    BOOST_CHECK(typeid(subgroup_container::iterator::difference_type) ==  typeid(std::ptrdiff_t));

    BOOST_CHECK(typeid(dataset_container::iterator::reference) == typeid(dataset&));
    BOOST_CHECK(typeid(subgroup_container::iterator::reference) == typeid(group&));

    BOOST_CHECK(typeid(dataset_container::iterator::pointer) == typeid(std::unique_ptr<dataset>));
    BOOST_CHECK(typeid(subgroup_container::iterator::pointer) == typeid(std::unique_ptr<group>));

    BOOST_CHECK(typeid(dataset_container::iterator::iterator_category) == typeid(std::forward_iterator_tag));
    BOOST_CHECK(typeid(subgroup_container::iterator::iterator_category) == typeid(std::forward_iterator_tag));

    /** setting up further test with a real group */
    group container_group(file);
    dataset dset1 = create_dataset<int>(container_group, "dset1");
    dataset dset2 = create_dataset<int>(container_group, "dset2");
    dataset dset3 = create_dataset<int>(container_group, "dset3");
    group grp1(container_group, "grp1");
    group grp2(container_group, "grp2");
    group grp3(container_group, "grp3");

    dataset_container::iterator dset_multipass_1 = container_group.datasets().begin();
    dataset_container::iterator dset_multipass_2 = container_group.datasets().begin();
    subgroup_container::iterator sgroup_multipass_1 = container_group.subgroups().begin();
    subgroup_container::iterator sgroup_multipass_2 = container_group.subgroups().begin();

    /** check Multipass guarantee */
    BOOST_TEST_MESSAGE("testing Multipass guarantee");
    while(dset_multipass_1 != container_group.datasets().end()) {
        BOOST_CHECK(dset_multipass_1 == dset_multipass_2);
	BOOST_CHECK(dset_multipass_1.get_current_name() == dset_multipass_2.get_current_name());
	++dset_multipass_1;
	dset_multipass_2++;
    }

    while(sgroup_multipass_1 != container_group.subgroups().end()) {
	BOOST_CHECK(sgroup_multipass_1 == sgroup_multipass_2);
	BOOST_CHECK(sgroup_multipass_1.get_current_name() == sgroup_multipass_2.get_current_name());
	++sgroup_multipass_1;
	sgroup_multipass_2++;
    }
}

BOOST_AUTO_TEST_CASE( iterator_expressions )
{
    BOOST_TEST_MESSAGE("\nTesting basic iterator expressions");

    container<dataset>::iterator dset_iter;
    container<dataset>::iterator dset_iter_2;
    container<group>::iterator sgroup_iter;
    container<group>::iterator sgroup_iter_2;

    /** check operator*
     *  should throw h5xx::error, since container_group_ does not exist
     */
    BOOST_TEST_MESSAGE("testing dereference operator");
    BOOST_CHECK_THROW(*sgroup_iter, std::invalid_argument);
    BOOST_CHECK_THROW(*dset_iter, std::invalid_argument);
 
    /** check operator++() */
    BOOST_TEST_MESSAGE("testing pre-increment operator");
    BOOST_CHECK_THROW(++dset_iter, std::invalid_argument);
    BOOST_CHECK_THROW(++sgroup_iter, std::invalid_argument);

    /** check operator++(int) */
    BOOST_TEST_MESSAGE("testing post-increment operator");
    BOOST_CHECK_THROW(dset_iter++, std::invalid_argument);
    BOOST_CHECK_THROW(sgroup_iter++, std::invalid_argument);

    /** TODO: check (void) i++ == (void)++i */


    /** check operator!= / operator== */
    BOOST_TEST_MESSAGE("testing (in)equality operators");
    BOOST_CHECK(dset_iter == dset_iter_2);
    BOOST_CHECK(!(dset_iter != dset_iter_2));
    BOOST_CHECK(sgroup_iter == sgroup_iter_2);
    BOOST_CHECK(!(sgroup_iter != sgroup_iter_2));

    /** TODO: check Multipass guarantee */


     /** check It->m equivalent to (*It).m */
    group container_group(file);
    dataset dset = create_dataset<int>(container_group, "dset");
    group sgroup = group(container_group, "grp");

    dset_iter = container_group.datasets().begin();
    sgroup_iter = container_group.subgroups().begin();

    BOOST_TEST_MESSAGE("testing equivalnece of operator-> and operator*/.");
    BOOST_CHECK(dset_iter->valid() == (*dset_iter).valid());
    BOOST_CHECK(sgroup_iter->valid() == (*sgroup_iter).valid());
    
}

BOOST_AUTO_TEST_CASE( default_group )
{
    BOOST_TEST_MESSAGE("\nTesting iterator over default constructed group");
    group container_group = group();

    BOOST_TEST_MESSAGE("setting up iterators");
    container<dataset>::iterator dset_iter_begin, dset_iter_end;
    container<group>::iterator sgroup_iter_begin, sgroup_iter_end;

    BOOST_CHECK_NO_THROW(dset_iter_begin = container_group.datasets().begin());
    BOOST_CHECK_NO_THROW(dset_iter_end = container_group.datasets().end());
    BOOST_CHECK_NO_THROW(sgroup_iter_begin = container_group.groups().begin());
    BOOST_CHECK_NO_THROW(sgroup_iter_end = container_group.groups().end());

    BOOST_TEST_MESSAGE("testing for equality of begin and end iterators");
    BOOST_CHECK(dset_iter_begin == dset_iter_end);
    BOOST_CHECK(sgroup_iter_begin == sgroup_iter_begin);

    BOOST_CHECK_THROW(*dset_iter_begin, std::out_of_range);
    BOOST_CHECK_THROW(*sgroup_iter_begin, std::out_of_range);
}

BOOST_AUTO_TEST_CASE( empty_group )
{
    BOOST_TEST_MESSAGE("\nTesting iterator over empty group");
    group container_group(file);

    BOOST_TEST_MESSAGE("setting up iterators");
    container<dataset>::iterator dset_iter_begin, dset_iter_end;
    container<group>::iterator sgroup_iter_begin, sgroup_iter_end;

    BOOST_CHECK_NO_THROW(dset_iter_begin = container_group.datasets().begin());
    BOOST_CHECK_NO_THROW(dset_iter_end = container_group.datasets().end());
    BOOST_CHECK_NO_THROW(sgroup_iter_begin = container_group.groups().begin());
    BOOST_CHECK_NO_THROW(sgroup_iter_end = container_group.groups().end());

    // begin- and end-iterator should be equal in empty group
    BOOST_TEST_MESSAGE("testing for equality of begin and end iterators");
    BOOST_CHECK(dset_iter_begin == dset_iter_end);
    BOOST_CHECK(sgroup_iter_begin == sgroup_iter_end);

    BOOST_CHECK_THROW(*dset_iter_begin, std::out_of_range);
    BOOST_CHECK_THROW(*sgroup_iter_begin, std::out_of_range);
}


BOOST_AUTO_TEST_CASE( only_datasets )
{
    BOOST_TEST_MESSAGE("\nTesting iterator over dataset-only group");
    group container_group(file);

    dataset dset1 = create_dataset<int>(container_group, "dset1");
    dataset dset2 = create_dataset<int>(container_group, "dset2");
    dataset dset3 = create_dataset<int>(container_group, "dset3");

    // testing begin(), end()
    BOOST_TEST_MESSAGE("testing begin/end");
    BOOST_CHECK_NO_THROW(container<dataset>::iterator dset_iter_begin = container_group.datasets().begin());
    BOOST_CHECK_NO_THROW(container<dataset>::iterator dset_iter_end = container_group.datasets().end());
    BOOST_CHECK_NO_THROW(container<group>::iterator sgroup_iter_begin = container_group.groups().begin());
    BOOST_CHECK_NO_THROW(container<group>::iterator sgroup_iter_end = container_group.groups().end());

    container<dataset>::iterator dset_iter_begin = container_group.datasets().begin();
    container<dataset>::iterator dset_iter_end = container_group.datasets().end();
    container<dataset>::iterator dset_iter_begin_2 = container_group.datasets().begin();

    container<group>::iterator sgroup_iter_begin = container_group.groups().begin();
    container<group>::iterator sgroup_iter_end = container_group.groups().end();

    // no subgroups: begin and end iterators over subgroups should be equal
    BOOST_TEST_MESSAGE("testing (in)equality begin and end iterators");
    BOOST_CHECK(sgroup_iter_begin == sgroup_iter_end);

    // begin and end iterators over datasets should not be equal
    BOOST_CHECK(dset_iter_begin != dset_iter_end);
    BOOST_CHECK(dset_iter_begin.get_name() == "dset1"); // FIXME: do we have a guarantee that the order is  maintained?
    BOOST_CHECK(dset_iter_begin == dset_iter_begin_2);

    // check operators / expressions FIXME to seperate test
    BOOST_TEST_MESSAGE("testing operator*()");
    BOOST_CHECK((*dset_iter_begin).valid());

    BOOST_TEST_MESSAGE("testing 1st increment operator");
    BOOST_CHECK((*dset_iter_begin).valid());
    BOOST_CHECK(dset_iter_begin++ != dset_iter_end);
    BOOST_CHECK(dset_iter_begin.get_name() == "dset2");

    BOOST_TEST_MESSAGE("testing 2nd increment operator");
    ++dset_iter_begin;
    BOOST_CHECK((*dset_iter_begin).valid());
    BOOST_CHECK(dset_iter_begin.get_name() == "dset3");
    ++dset_iter_begin;
    BOOST_CHECK(dset_iter_begin == dset_iter_end);

    BOOST_TEST_MESSAGE("testing increment operator of dset_iter_begin_2");
    BOOST_CHECK(dset_iter_begin != dset_iter_begin_2++);
    ++dset_iter_begin_2;
    ++dset_iter_begin_2;
    BOOST_CHECK(dset_iter_begin_2 == dset_iter_end);
}


BOOST_AUTO_TEST_CASE( only_subgroups )
{
    BOOST_TEST_MESSAGE("\nTesting iterator over subgroup-only group");
    group container_group(file);
    group grp1(container_group, "grp1");
    group grp2(container_group, "grp2");
    group grp3(container_group, "grp3");

    BOOST_TEST_MESSAGE("testing begin/end");
    BOOST_CHECK_NO_THROW(container<dataset>::iterator dset_iter_begin = container_group.datasets().begin());
    BOOST_CHECK_NO_THROW(container<dataset>::iterator dset_iter_end = container_group.datasets().end());
    BOOST_CHECK_NO_THROW(container<group>::iterator sgroup_iter_begin = container_group.groups().begin());
    BOOST_CHECK_NO_THROW(container<group>::iterator sgroup_iter_end = container_group.groups().end());

    container<dataset>::iterator dset_iter_begin = container_group.datasets().begin();
    container<dataset>::iterator dset_iter_end = container_group.datasets().end();

    container<group>::iterator sgroup_iter_begin = container_group.groups().begin();
    container<group>::iterator sgroup_iter_end = container_group.groups().end();
    container<group>::iterator sgroup_iter_begin_2 = container_group.groups().begin();

    // begin and end iterator over subgroups should not be equal
    BOOST_TEST_MESSAGE("testing (in)equality begin and end iterators");
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_end);
    BOOST_CHECK(sgroup_iter_begin == sgroup_iter_begin_2++);
    BOOST_CHECK(sgroup_iter_begin.get_name() == "grp1");

    // begin- and end-iterator over datasets should be equal
    BOOST_CHECK(dset_iter_begin == dset_iter_end);

    // check operators
    BOOST_TEST_MESSAGE("testing operator*()");
    BOOST_CHECK((*sgroup_iter_begin).valid());

    BOOST_TEST_MESSAGE("testing 1st increment operator");
    ++sgroup_iter_begin;
    BOOST_CHECK((*sgroup_iter_begin).valid());
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_end);
    BOOST_CHECK(sgroup_iter_begin.get_name() == "grp2");

    BOOST_TEST_MESSAGE("testing 2nd increment operator");
    ++sgroup_iter_begin;
    BOOST_CHECK((*sgroup_iter_begin).valid());
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_end);
    BOOST_CHECK(sgroup_iter_begin.get_name() == "grp3");

    BOOST_TEST_MESSAGE("testing 3rd increment operator");
    ++sgroup_iter_begin;
    BOOST_CHECK(sgroup_iter_begin == sgroup_iter_end);

    BOOST_TEST_MESSAGE("testing increment operator of sgroup_iter_begin_2");
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_begin_2);
    ++sgroup_iter_begin_2;
    ++sgroup_iter_begin_2;
    BOOST_CHECK(sgroup_iter_begin_2 == sgroup_iter_end);
}


BOOST_AUTO_TEST_CASE( mixed_1 )
{
    BOOST_TEST_MESSAGE("\nTesting iterator over 1st mixed group");
    /** container group with 2 datasets and 1 subgroup **/
    group container_group(file);
    group grp1(container_group, "grp1");
    dataset dset1 = create_dataset<int>(container_group, "dset1");
    dataset dset2 = create_dataset<int>(container_group, "dset2");

    BOOST_TEST_MESSAGE("testing begin/end");
    BOOST_CHECK_NO_THROW(container<dataset>::iterator dset_iter_begin = container_group.datasets().begin());
    BOOST_CHECK_NO_THROW(container<dataset>::iterator dset_iter_end = container_group.datasets().end());
    BOOST_CHECK_NO_THROW(container<group>::iterator sgroup_iter_begin = container_group.groups().begin());
    BOOST_CHECK_NO_THROW(container<group>::iterator sgroup_iter_end = container_group.groups().end());

    container<dataset>::iterator dset_iter_begin = container_group.datasets().begin();
    container<dataset>::iterator dset_iter_end = container_group.datasets().end();
    container<group>::iterator sgroup_iter_begin = container_group.groups().begin();
    container<group>::iterator sgroup_iter_end = container_group.groups().end();

    BOOST_TEST_MESSAGE("testing (in)equality of begin and end iterators");
    // begin- and end-iterator over subgroups should not be equal
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_end);
    BOOST_CHECK(sgroup_iter_begin.get_name() == "grp1");

    // begin- and end-iterator over datasets should not be equal
    BOOST_CHECK(dset_iter_begin != dset_iter_end);
    BOOST_CHECK(dset_iter_begin.get_name() == "dset1");

    // iterate subgroup iter
    BOOST_TEST_MESSAGE("testing increment operator of subgroup iterator");
    BOOST_CHECK((*sgroup_iter_begin++).valid());
    BOOST_CHECK(sgroup_iter_begin == sgroup_iter_end);

    // iterate dataset iter
    BOOST_TEST_MESSAGE("testing increment operator of dataset iterator");
    BOOST_CHECK((*dset_iter_begin++).valid());
    BOOST_CHECK(dset_iter_begin != dset_iter_end);
    BOOST_CHECK(dset_iter_begin.get_name() == "dset2");
    dset_iter_begin++;
    BOOST_CHECK(dset_iter_begin == dset_iter_end);
}


BOOST_AUTO_TEST_CASE( mixed_2 )
{
    BOOST_TEST_MESSAGE("\nTesting iterator over 2nd mixed group");
    /** container group with 1 dataset and 2 subgroups **/
    group container_group(file);
    group grp1(container_group, "grp1");
    group grp2(container_group, "grp2");
    dataset dset1 = create_dataset<int>(container_group, "dset1");
    dataset temp_set;
    group temp_group;

    BOOST_TEST_MESSAGE("testing begin/end");
    BOOST_CHECK_NO_THROW(container<dataset>::iterator dset_iter_begin = container_group.datasets().begin());
    BOOST_CHECK_NO_THROW(container<dataset>::iterator dset_iter_end = container_group.datasets().end());
    BOOST_CHECK_NO_THROW(container<group>::iterator sgroup_iter_begin = container_group.groups().begin());
    BOOST_CHECK_NO_THROW(container<group>::iterator sgroup_iter_end = container_group.groups().end());

    container<dataset>::iterator dset_iter_begin = container_group.datasets().begin();
    container<dataset>::iterator dset_iter_end = container_group.datasets().end();
    container<group>::iterator sgroup_iter_begin = container_group.groups().begin();
    container<group>::iterator sgroup_iter_end = container_group.groups().end();

    BOOST_TEST_MESSAGE("testing (in)equality of begin and end iterators");
    // begin- and end-iterator over subgroups should not be equal
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_end);
    BOOST_CHECK(sgroup_iter_begin.get_name() == "grp1");

    // begin- and end-iterator over datasets should not be equal
    BOOST_CHECK(dset_iter_begin != dset_iter_end);
    BOOST_CHECK(dset_iter_begin.get_name() == "dset1");

    // iterate subgroup iter
    BOOST_TEST_MESSAGE("testing increment operator of subgroup iterator");
    BOOST_CHECK((*sgroup_iter_begin++).valid());
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_end);
    BOOST_CHECK(sgroup_iter_begin.get_name() == "grp2");

    // iterate dataset iter
    BOOST_TEST_MESSAGE("testing increment operator of dataset iterator");
    BOOST_CHECK((*dset_iter_begin++).valid());
    BOOST_CHECK(dset_iter_begin == dset_iter_end);
}


BOOST_AUTO_TEST_CASE( mixed_3 )
{
    BOOST_TEST_MESSAGE("\nTesting iterator over 3rd mixed group");
    /** container group with 2 datasets and 2 subgroups **/
    group container_group(file);
    group grp1(container_group, "grp1");
    group grp2(container_group, "grp2");
    dataset dset1 = create_dataset<int>(container_group, "dset1");
    dataset dset2 = create_dataset<int>(container_group, "dset2");

    BOOST_TEST_MESSAGE("testing begin/end");
    BOOST_CHECK_NO_THROW(container<dataset>::iterator dset_iter_begin = container_group.datasets().begin());
    BOOST_CHECK_NO_THROW(container<dataset>::iterator dset_iter_end = container_group.datasets().end());
    BOOST_CHECK_NO_THROW(container<group>::iterator sgroup_iter_begin = container_group.groups().begin());
    BOOST_CHECK_NO_THROW(container<group>::iterator sgroup_iter_end = container_group.groups().end());

    container<dataset>::iterator dset_iter_begin = container_group.datasets().begin();
    container<dataset>::iterator dset_iter_end = container_group.datasets().end();
    container<group>::iterator sgroup_iter_begin = container_group.groups().begin();
    container<group>::iterator sgroup_iter_end = container_group.groups().end();

    BOOST_TEST_MESSAGE("testing (in)equality of begin and end iterators");
    // begin- and end-iterator over subgroups should not be equal
    BOOST_TEST_MESSAGE("testing (in)equality of begin and end iterators");
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_end);
    BOOST_CHECK(sgroup_iter_begin.get_name() == "grp1");

    // begin- and end-iterator over datasets should not be equal
    BOOST_CHECK(dset_iter_begin != dset_iter_end);
    BOOST_CHECK(dset_iter_begin.get_name() == "dset1");

    // iterate both iterators
    BOOST_TEST_MESSAGE("testing increment dataset/subgroup iterators alternateingly");
    BOOST_CHECK((*sgroup_iter_begin++).valid());
    BOOST_CHECK(sgroup_iter_begin != sgroup_iter_end);
    BOOST_CHECK(sgroup_iter_begin.get_name() == "grp2");

    BOOST_CHECK((*dset_iter_begin++).valid());
    BOOST_CHECK(dset_iter_begin != dset_iter_end);
    BOOST_CHECK(dset_iter_begin.get_name() == "dset2");

    dset_iter_begin++;
    sgroup_iter_begin++;

    BOOST_CHECK(sgroup_iter_begin == sgroup_iter_end);
    BOOST_CHECK(dset_iter_begin == dset_iter_end);
}
} // namespace fixture
