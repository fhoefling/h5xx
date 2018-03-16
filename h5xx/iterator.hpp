/**
 *  Copyright © 2017-2018      Matthias Werner and Felix Höfling
 *  All rights reserved.
 *
 *  This file is part of h5xx — a C++ wrapper for the HDF5 library.
 *
 *  This software may be modified and distributed under the terms of the
 *  3-clause BSD license.  See accompanying file LICENSE for details.
 */

#ifndef H5XX_ITERATOR_HPP
#define H5XX_ITERATOR_HPP

#include <h5xx/hdf5_compat.hpp>
#include <h5xx/group.hpp>
#include <h5xx/dataset.hpp>


namespace h5xx
{

/**
 *  This class implements a forward-iterator for h5xx::group.
 *  Via a template parameter the type of element to iterate over is determined
 *  So far possible types are h5xx::group, for iteration over subgroups, and
 *  h5xx::dataset, for iteration over the datasets within the group.
 */

template <typename T>
class iterator : public std::iterator<std::forward_iterator_tag, T>
{

public:

    iterator(group&);
    iterator(const iterator&);
    ~iterator();

    /** increment and decrement operators move to next element **/
    iterator& operator++();
    iterator& operator++(int);

/*    iterator& operator--();
    iterator& operator--(int);
*/
    /** returns h5xx-object **/
    T operator*();

    /** comparison operators **/
    /** determined on basis of hdf5 id **/
    bool operator==(const iterator&) const;
    bool operator!=(const iterator&) const;
/*
    str::string const& name() const
    {   return name_of_current_element;
    }
*/

private:

    /** pointer to container group **/
    h5xx::group* container_group;

    /** index of element in group as used by H5Literate **/
    // if stop_idx == -1, iterator points to end
    hsize_t stop_idx;

    /** name of element pointed to **/
    std::string name_of_current_element;

}; // class iterator


// FIXME the same again for const_iterator and "const T", compare with /usr/include/c++/4.9.2/bits/stl_list.h

namespace detail {

// TODO: is there a better way using more h5xx than hdf5?
template <typename T>
herr_t find_name_of_type(hid_t g_id, const char* name, const H5L_info_t *info, void *op_data) noexcept;

} // namespace detail


template <typename T>
inline iterator<T>::iterator(group& group) : container_group(&group)
{
    stop_idx = -1u;  // constructor gives end()-iterator by default
}


template <typename T>
inline iterator<T>::iterator(const iterator& other) : container_group(other.container_group)
{
    name_of_current_element = other.name_of_current_element;
    stop_idx = other.stop_idx;
}


template <typename T>
inline iterator<T>::~iterator(){}


template <typename T>
inline bool iterator<T>::operator==(const iterator& other)
{
    // compare string as well
    if(stop_idx == other.stop_idx && container_group == other.container_group)
        return true;
    else
        return false;
}


template <typename T>
inline bool iterator<T>::operator!=(const iterator& other)
{
    if(stop_idx == other.stop_idx && container_group == other.container_group)
        return false;
    else
        return true;
}


template <typename T>
inline iterator<T>& iterator<T>::operator++() // ++it
{
    herr_t retval = H5Literate(container_group->hid(), H5_INDEX_NAME, H5_ITER_INC, &stop_idx, detail::find_name_of_type<T>, &name_of_current_element); // FIXME: H5_ITER_NATIVE faster?? see alse operator-- / operator*
    
    // evaluate return value
    // retval is return-value of operator (usually > 0), == 0 iff all elements have been iterated over with no non-zero operator
    // on failure or if operator returns negative, returns the negative
    if(retval < 0) // there has been an error upon iteration or reading of  object info
    {
        throw std::run:time_error("Error within H5Literate or detail::find_name_of_type")
    }
    else if(retval == 0) // there is no more elements of T in container group
    {
        throw std::out_of_range("Iterator out of range");  // set to 'invalid' iterator, e.g., stop_idx = -1
    }
      
    return(*this);
}


template <typename T>
inline iterator<T>& iterator<T>::operator++(int) // it++
{
    ++(*this);
    return(*this);
}


template <typename T>
inline iterator<T>& iterator<T>::operator--() // --it
{
    herr_t retval = H5Literate(container_group->hid(), H5_INDEX_NAME, H5_ITER_DEC, &stop_idx, detail::find_name_of_type<T>, &name_of_current_element);
    
    // evaluate return value
    // retval is return-value of operator (usually > 0), == 0 iff all elements have been iterated over with no non-zero operator
    // on failure or if operator returns negative, returns the negative
    if(retval < 0) // there has been an error upon iteration or reading of  object info
    {
        throw std::run:time_error("Error within H5Literate or detail::find_name_of_type")
    }
    else if(retval == 0) // there is no more elements of T in container group
    {
        throw std::out_of_range("Iterator out of range");
    }
      
    return(*this);
}


template <typename T>
inline iterator<T>& iterator<T>::operator--(int) // it--
{
    --(*this);
    return(*this);
}

template <>
h5xx::group iterator<h5xx::group>::operator*()
{
    if (stop_idx == -1u) // iterator is end
    {
        stop_idx = 0;
        herr_t retval = H5Literate(container_group->hid(), H5_INDEX_NAME, H5_ITER_INC, &stop_idx, detail::find_name_of_type<h5xx::group>, &name_of_current_element);
        
        // evaluate retval
        // positive retval -> all is good
        if(retval < 0) // negative retval means error in H5Literate
        {
            throw std::run:time_error("Error within H5Literate or detail::find_name_of_type");
        }
        else // retval == 0 means no element of type T was found
        {
            stop_idx = -1u; // remain at end
            // FIXME: what else to do when end-iterator gets dereferenced? (e.g. std::vector end-iterator can dereferenced, although might get seg fault at runtime)
        }
    }

    h5xx::group group(container_group_id, name_of_current_element);
    return(h5xx::move(group));
}


template <>
h5xx::dataset iterator<h5xx::dataset>::operator*()
{
    if (stop_idx == -1u) // iterator is end
    {
        stop_idx = 0;
        herr_t retval = H5Literate(container_group->hid(), H5_INDEX_NAME, H5_ITER_INC, &stop_idx, detail::find_name_of_type<h5xx::dataset>, &name_of_current_element);
        
        // evaluate retval
        // positive retval -> all is good
        if(retval < 0) // negative retval means error in H5Literate
        {
            throw std::run:time_error("Error within H5Literate or detail::find_name_of_type");
        }
        else // retval == 0 means no element of type T was found
        {
            stop_idx = -1u; // remain at end
            // FIXME: what else to do when end-iterator gets dereferenced? (e.g. std::vector end-iterator can dereferenced, although might get seg fault at runtime)
        }
    }

    h5xx::dataset dset(container_group, *internal_iter);
    return(h5xx::move(dset));
}


namespace detail {

template <>
herr_t find_name_of_type<h5xx::group>(hid_t g_id, const char* name, const H5L_info_t *info, void *op_data) noexcept
{
    H5O_info_t obj_info;
    herr_t retval = H5Oget_info_by_name(g_id, name, &obj_info, H5P_DEFAULT); // returns non-negative upon success, negative if failed
    
    // check retval
    if(retval >= 0)
    {   
        // filter for groups
        if(obj_info.type == H5O_TYPE_GROUP)
        {
            std::string *str_ptr = reinterpret_cast<std::string *>(op_data);
            *str_ptr = name;
            retval++; // ensure retval is > 0 for short-circuit success
        }
        else // if element name was not a group, continue iteration
            retval = 0;
    }
    
    return(retval);
}

template <>
herr_t find_name_of_type<h5xx::dataset>(hid_t g_id, const char* name, const H5L_info_t *info, void *op_data) noexcept
{
    H5O_info_t obj_info;
    herr_t retval = H5Oget_info_by_name(g_id, name, &obj_info, H5P_DEFAULT);
    
    if(retval >= 0)
    {
        // filter for datasets
        if(obj_info.type == H5O_TYPE_DATASET)
        {
            std::string *str_ptr = reinterpret_cast<std::string *>(op_data);
            *str_prt = name;
            retval++; // ensure retval is > 0 for short-circuit success
        }
        else // if element name was not a dataset, continue iteration
            retval = 0;
    }

    return(retval);
}

} // namespace detail

} // namespace h5xx

#endif // ! H5XX_ITERATOR_HPP
