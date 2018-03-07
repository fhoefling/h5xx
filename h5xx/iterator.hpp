/**
 *  Copyright © 2018      Matthias Werner and Felix Höfling
 *  All rights reserved.
 *
 *  This file is part of h5xx — a C++ wrapper for the HDF5 library.
 */

#ifndef H5XX_ITERATOR_HPP
#define H5XX_ITERATOR_HPP

#include <h5xx/hdf5_compat.hpp>  // FIXME: I guess this will do as include hdf5.h?
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

	//iterator operator+(int);
    //iterator operator-(int);

	iterator operator+=(int);
	iterator operator-=(int);

    /** increment and decrement operators move to next element **/
    iterator& operator++();
    iterator& operator++(int);

    iterator& operator--();
    iterator& operator--(int);

    /** returns h5xx-object **/
    const T operator*() const; // FIXME: const T -> dereferenced object can't be altered? T == h5xx::dataset could not be read/written to

    /** comparison operators **/
    /** determined on basis of hdf5 id **/
    bool operator==(const iterator&) const;
    bool operator!=(const iterator&) const;


private:

    /** id of container group **/
    const h5xx::group container_group;

    /** index of element in group as used by H5Literate **/
    hsize_t stop_idx;

    /** name of element pointed to **/
    std::string name_of_current_element;

}; // class iterator


namespace detail {

// TODO: is there a better way using more h5xx than hdf5?
template <typename T>
herr_t find_names_of_type(hid_t g_id, const char* name, const H5L_info_t *info, void *op_data) noexcept;

} // namespace detail


template <typename T>
inline iterator<T>::iterator(group& group) : container_group(group)
{
    stop_idx = 0;
    
    // find first element in group
    herr_t retval = H5Literate(container_group.hid(), H5_INDEX_NAME, H5_ITER_INC, &stop_idx, detail::find_name_of_type<T>, &name_of_current_element);    
}


template <typename T>
inline iterator<T>::iterator(const iterator& other) : container_group(other.container_group)
{
    name_of_current_element = other.name_of_current_element;
    stop_idx = other.stop_idx;
}


template <typename T>
inline iterator<T>::~iterator(){}


// FIXME == and != operator must be implemented for h5xx::group
template <typename T>
inline bool iterator<T>::operator==(const iterator& other)
{
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
    herr_t retval = H5Literate(container_group.hid(), H5_INDEX_NAME, H5_ITER_INC, &stop_idx, detail::find_name_of_type<T>, &name_of_current_element); // FIXME: H5_ITER_NATIVE faster?? see alse operator--
    
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
inline iterator<T>& iterator<T>::operator++(int) // it++
{
    ++(*this);
    return(*this);
}


template <typename T>
inline iterator<T>& iterator<T>::operator--() // --it
{
    herr_t retval = H5Literate(container_group.hid(), H5_INDEX_NAME, H5_ITER_DEC, &stop_idx, detail::find_name_of_type<T>, &name_of_current_element);
    
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
    h5xx::group group(container_group_id, name_of_current_element);
    return(h5xx::move(group));
}


template <>
h5xx::dataset iterator<h5xx::dataset>::operator*()
{
    h5xx::dataset dset(container_group, *internal_iter);
    return(h5xx::move(dset));
}


namespace detail {

// TODO: sind diese templates überhaupt sinnvoll?? was passiert bei anderen Werten der Template-Parameter ??
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
