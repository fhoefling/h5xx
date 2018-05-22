/*
 * Copyright © 2008-2013 Felix Höfling
 * Copyright © 2014-2015 Klaus Reuter
 * Copyright © 2013      Manuael Dibak
 * Copyright © 2008-2010 Peter Colberg
 * All rights reserved.
 *
 * This file is part of h5xx — a C++ wrapper for the HDF5 library.
 *
 * This software may be modified and distributed under the terms of the
 * 3-clause BSD license.  See accompanying file LICENSE for details.
 */

#ifndef H5XX_GROUP_HPP
#define H5XX_GROUP_HPP

#include <h5xx/file.hpp>
#include <h5xx/dataset.hpp>
#include <h5xx/utility.hpp>
#include <h5xx/error.hpp>
#include <h5xx/property.hpp>

namespace h5xx {

/** forward declaration **/
class dataset_container;
class subgroup_container;

// this class is meant to replace the H5::Group class
class group
{
public:
    /** default constructor */
    group() : hid_(-1) {}

    /** constructor to open or generate a group in an existing superior group */
    group(group const& other, std::string const& name);

    /** open root group of file */
    group(file const& f);

    /**
     * deleted copy constructor
     *
     * Calling the constructor throws an exception. Copying must be elided by
     * return value optimisation. See also "group h5xx::move(group&)".
     */
    group(group const& other);

    /**
     * assignment operator
     *
     * Uses the copy-and-swap idiom. Move semantics is achieved in conjunction
     * with "group h5xx::move(group&)", i.e., the group object on the right
     * hand side is empty after move assignment.
     */
    group & operator=(group other);

    /** default destructor */
    ~group();

    /** return HDF5 object ID */
    hid_t hid() const
    {
        return hid_;
    }

    /** returns true if associated to a valid HDF5 object */
    bool valid() const
    {
        return hid_ >= 0;
    }

    /** open handle to HDF5 group from an object's ID (called by non-default constructors) */
    void open(group const& other, std::string const& name);

    /** close handle to HDF5 group (called by default destructor) */
    void close();
    
    /** methods for iterator-adapters **/
    dataset_container datasets();
    subgroup_container subgroups();


private:
    /** HDF5 object ID */
    hid_t hid_;

    template <typename h5xxObject>
    friend void swap(h5xxObject& left, h5xxObject& right);
}; // class group

/** iterator template class **/
template <typename T>
class group_iterator : public std::iterator<std::forward_iterator_tag, T>
{

public:

    //FIXME: swappable?
    //FIXME: noexcept in constructor?
    group_iterator() noexcept;
    group_iterator(const group&) noexcept;
    group_iterator(const group_iterator&) noexcept;
    ~group_iterator() {};

    /** pre- and post-increment operators */
    group_iterator& operator++();
    group_iterator operator++(int);

    /** returns h5xx-object */
    T operator*();
    //T* operator->():

    /** comparison operators
     *  determined on basis of hdf5 id
     */
    bool operator==(group_iterator const&) const;
    bool operator!=(group_iterator const&) const;

    /** return name of current element
     * just for testing 
     */
    std::string get_current_name()
    {
        return(name_of_current_element);
    };

    /** initialize iterator
     *  as past-the-end
     */
    void set_to_end_() noexcept;


private:

    /** move forward by one step, call H5Literate */
    bool increment_();
 
    /** pointer to container group */
    group const* container_group_;

    /** index of element in group as used by H5Literate
     * if stop_idx == -1, iterator points to end
     */
    hsize_t stop_idx;

    /** name of element pointed to */
    std::string name_of_current_element;

}; // class group_iterator


// FIXME the same again for const_group_iterator and "const T", compare with /usr/include/c++/4.9.2/bits/stl_list.h

namespace detail {

// TODO: is there a better way using more h5xx than hdf5?
template <typename T>
herr_t find_name_of_type(hid_t g_id, const char* name, const H5L_info_t* info, void* op_data);

template <H5O_type_t>
herr_t find_name_of_type_impl(hid_t g_id, const char* name, const H5L_info_t* info, void* op_data);

} // namespace detail


/** 
 * adapter classes for iterators
 */
class dataset_container
{

public:
    
    typedef group_iterator<dataset> iterator;	
    typedef group_iterator<dataset const> const_iterator;
    dataset_container(const group&);

    iterator begin() noexcept;
    iterator end() noexcept;

    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;

private:

    group const* container_group_;
};

class subgroup_container
{

public:
    
    typedef group_iterator<group> iterator;
    typedef group_iterator<group const> const_iterator;
    subgroup_container(const group&);

    iterator begin() noexcept;
    iterator end() noexcept;
    
    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;

private:
    
    group const* container_group_;
};


/**
 * return true if group "name" exists in group "grp"
 */
inline bool exists_group(group const& grp, std::string const& name);

inline group::group(group const& other, std::string const& name)
  : hid_(-1)
{
    open(other, name);
}

inline group::group(file const& f)
{
    hid_ = H5Gopen(f.hid(), "/", H5P_DEFAULT);
    if (hid_ < 0){
        throw error("opening root group of file \"" + f.name() + "\"");
    }
}

inline group::group(group const& other)
  : hid_(other.hid_)
{
    // copying would be safe if the exception were disabled.
    throw error("h5xx::group can not be copied. Copying must be elided by return value optimisation.");
    H5Iinc_ref(hid_);
}

inline group & group::operator=(group other)
{
    swap(*this, other);
    return *this;
}

inline group::~group()
{
    close();
}

inline void group::open(group const& other, std::string const& name)
{
    if (hid_ >= 0) {
        throw error("h5xx::group object is already in use");
    }

    if (exists_group(other, name)) {
        hid_ = H5Gopen(other.hid(), name.c_str(), H5P_DEFAULT);
    }
    else {
        hid_t lcpl_id = H5Pcreate(H5P_LINK_CREATE);     // create group creation property list
        H5Pset_create_intermediate_group(lcpl_id, 1);   // set intermediate link creation
        hid_ = H5Gcreate(other.hid(), name.c_str(), lcpl_id, H5P_DEFAULT, H5P_DEFAULT);
    }
    if (hid_ < 0){
        throw error("creating or opening group \"" + name + "\"");
    }
}

inline void group::close() {
    if (hid_ >= 0) {
        if(H5Gclose(hid_) < 0){
            throw error("closing h5xx::group with ID " + boost::lexical_cast<std::string>(hid_));
        }
        hid_ = -1;
    }
}

/**
 * return true if group "name" exists in group "grp"
 */
inline bool exists_group(group const& grp, std::string const& name)
{
    hid_t hid = grp.hid();
    H5E_BEGIN_TRY {
        hid = H5Gopen(hid, name.c_str(), H5P_DEFAULT);
        if (hid > 0) {
            H5Gclose(hid);
        }
    } H5E_END_TRY
    return (hid > 0);
}

inline hid_t open_group(hid_t loc_id, std::string const& path)
{
    hid_t group_id;
    H5E_BEGIN_TRY {
        group_id = H5Gopen(loc_id, path.c_str(), H5P_DEFAULT);
    } H5E_END_TRY
    return group_id;
}

inline dataset_container group::datasets()
{
    dataset_container container(*this);
    return(container);
}

inline subgroup_container group::subgroups()
{
    subgroup_container container(*this);
    return(container);
}

inline dataset_container::dataset_container(const group& grp) : container_group_(&grp) {}

inline dataset_container::iterator dataset_container::begin() noexcept
{
    iterator iter(*container_group_);
    return(iter);
}

inline dataset_container::iterator dataset_container::end() noexcept
{
    iterator iter(*container_group_);
    iter.set_to_end_();
    return(iter);
}

inline dataset_container::const_iterator dataset_container::cbegin() const noexcept
{
    const_iterator iter(*container_group_);
    return(iter);
}

inline dataset_container::const_iterator dataset_container::cend() const noexcept
{
    const_iterator iter(*container_group_);
    iter.set_to_end_();
    return(iter);
}

inline subgroup_container::subgroup_container(const group& grp) : container_group_(&grp) {}

inline subgroup_container::iterator subgroup_container::begin() noexcept
{
    iterator iter(*container_group_);
    return(iter);
}

inline subgroup_container::iterator subgroup_container::end() noexcept
{
    iterator iter(*container_group_);
    iter.set_to_end_();
    return(iter);
}

inline subgroup_container::const_iterator subgroup_container::cbegin() const noexcept
{
    const_iterator iter(*container_group_);
    return(iter);
}

inline subgroup_container::const_iterator subgroup_container::cend() const noexcept
{
    const_iterator iter(*container_group_);
    iter.set_to_end_();
    return(iter);
}

namespace detail{

template <>
herr_t find_name_of_type<group>(hid_t g_id, const char* name, const H5L_info_t *info, void *op_data)
{
    return(find_name_of_type_impl<H5O_TYPE_GROUP>(g_id, name, info, op_data));
}

template <>
herr_t find_name_of_type<group const>(hid_t g_id, const char* name, const H5L_info_t *info, void *op_data)
{
    return(find_name_of_type_impl<H5O_TYPE_GROUP>(g_id, name, info, op_data));
}

template <>
herr_t find_name_of_type<dataset>(hid_t g_id, const char* name, const H5L_info_t *info, void *op_data)
{
    return(find_name_of_type_impl<H5O_TYPE_DATASET>(g_id, name, info, op_data));
}

template <>
herr_t find_name_of_type<dataset const>(hid_t g_id, const char* name, const H5L_info_t *info, void *op_data)
{
    return(find_name_of_type_impl<H5O_TYPE_DATASET>(g_id, name, info, op_data));
}

template <H5O_type_t type>
herr_t find_name_of_type_impl(hid_t g_id, const char* name, const H5L_info_t *info, void *op_data)
{
    H5O_info_t obj_info;

    /** returns non-negative upon success, negative if failed */
    herr_t retval = H5Oget_info_by_name(g_id, name, &obj_info, H5P_DEFAULT);
    
    /** check retval */
    if(retval >= 0)
    {   
        /** filter for groups */
        if(obj_info.type == type)
        {
            std::string *str_ptr = reinterpret_cast<std::string *>(op_data);
            *str_ptr = name;
            retval++; // ensure retval is > 0 for short-circuit success
        }
        else // if element name was not a group, continue iteration
            retval = 0;
    }
    else
    {
        throw std::runtime_error("Error when getting object info of "+std::string(name));
    }
    
    return(retval);
}
} // namespace detail

template <typename T>
inline group_iterator<T>::group_iterator() noexcept
{
    stop_idx = -1U;
    container_group_ = NULL;
}

template <typename T>
inline group_iterator<T>::group_iterator(const group& group) noexcept : container_group_(&group)
{
    stop_idx = 0;
}


template <typename T>
inline group_iterator<T>::group_iterator(const group_iterator& other) noexcept
  : container_group_(other.container_group_)
  , stop_idx(other.stop_idx)
  , name_of_current_element(other.name_of_current_element)
{}

template <typename T>
inline bool group_iterator<T>::operator==(const group_iterator& other) const
{
    if (stop_idx == 0) {
        const_cast<group_iterator*>(this)->increment_();
    }
    
    if (other.stop_idx == 0) {
        const_cast<group_iterator*>(&other)->increment_();
    }
    
    return stop_idx == other.stop_idx;
}


template <typename T>
inline bool group_iterator<T>::operator!=(const group_iterator& other) const
{
    return !(*this == other);
}


template <typename T>
inline group_iterator<T>& group_iterator<T>::operator++() // ++it
{
    if (container_group_ == NULL) {
        throw std::invalid_argument("group_iterator was default constructed; doesn't point to a group");
    }

    if(stop_idx == 0)
        increment_();

    bool out_of_range = !increment_();

    // evaluate return value
    if(out_of_range)
        stop_idx = -1u;

    return(*this);
}


template <typename T>
inline group_iterator<T> group_iterator<T>::operator++(int) // it++
{
    group_iterator<T> this_temp(*this);
    ++(*this);
    return(this_temp); // FIXME: returns reference to local variable
}

template <typename T>
inline bool group_iterator<T>::increment_()
{
    /** check if container_group is valid
     *  if not, set iterator past the end
     *  and return false
     *
     *  FIXME: is this good style?
     */
    if(container_group_->hid() < 0){
        stop_idx = -1U;
	return false;
    }

    herr_t retval = H5Literate(
        container_group_->hid(), H5_INDEX_NAME, H5_ITER_INC, &stop_idx
      , detail::find_name_of_type<T>, &name_of_current_element
    );
    
    // evaluate return code
    if(retval == 0) // no element of type T was found
    {
        stop_idx = -1U; // set iterator to past-the-end iterator
        name_of_current_element = std::string();
    }
    
    return retval > 0;  // all is good
}

template <typename T>
inline T group_iterator<T>::operator*()
{
    bool out_of_range = (stop_idx == -1U); // iterator is past the end

    if (container_group_ == NULL) {
        throw std::invalid_argument("group_iterator was default constructed; doesn't point to a group");
    }

    if (stop_idx == 0) {
        out_of_range = !increment_();
    } 

    if (out_of_range) {
        
        std::string error_msg = std::string();

        if (container_group_ != NULL)
            error_msg = "dereference iterator without group";
	else
            error_msg = "container_group "+get_name(*container_group_);

        throw std::out_of_range(error_msg);
    }	
    
    T retval(*container_group_, name_of_current_element);
    return(move(retval));
}

template <typename T>
inline void group_iterator<T>::set_to_end_() noexcept
{
    stop_idx = -1U;
}

} // namespace h5xx

#endif /* ! H5XX_GROUP_HPP */
