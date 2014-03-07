/*
 * Copyright © 2008-2013 Felix Höfling
 * Copyright © 2013      Manuael Dibak
 * Copyright © 2008-2010 Peter Colberg
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

#ifndef H5XX_GROUP_HPP
#define H5XX_GROUP_HPP

#include <h5xx/file.hpp>
#include <h5xx/utility.hpp>
#include <h5xx/error.hpp>
#include <h5xx/property.hpp>

namespace h5xx {

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
    group const& operator=(group other);

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

private:
    /** HDF5 object ID */
    hid_t hid_;

    template <typename h5xxObject>
    friend void swap(h5xxObject& left, h5xxObject& right);
};

/**
 * return true if group "name" exists in group "grp"
 */
inline bool exists_group(group const& grp, std::string const& name);

group::group(group const& other, std::string const& name)
  : hid_(-1)
{
    open(other, name);
}

group::group(file const& f)
{
    hid_ = H5Gopen(f.hid(), "/", H5P_DEFAULT);
    if (hid_ < 0){
        throw error("opening root group of file \"" + f.name() + "\"");
    }
}

group::group(group const& other)
  : hid_(other.hid_)
{
    // copying would be safe if the exception were disabled.
    throw error("h5xx::group can not be copied. Copying must be elided by return value optimisation.");
    H5Iinc_ref(hid_);
}

group const& group::operator=(group other)
{
    swap(*this, other);
    return *this;
}

group::~group()
{
    close();
}

void group::open(group const& other, std::string const& name)
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

void group::close() {
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

inline bool exists_group(H5::CommonFG const& fg, std::string const& name)
{
    H5::IdComponent const& loc(dynamic_cast<H5::IdComponent const&>(fg));
    hid_t hid;
    H5E_BEGIN_TRY {
        hid = H5Gopen(loc.getId(), name.c_str(), H5P_DEFAULT);
        if (hid > 0) {
            H5Gclose(hid);
        }
    } H5E_END_TRY
    return (hid > 0);
}

/**
 * open or create HDF5 group
 *
 * This function creates missing intermediate groups.
 */
inline H5::Group open_group(H5::CommonFG const& fg, std::string const& path)
{
    H5::IdComponent const& loc(dynamic_cast<H5::IdComponent const&>(fg));
    hid_t group_id;
    H5E_BEGIN_TRY {
        group_id = H5Gopen(loc.getId(), path.c_str(), H5P_DEFAULT);
    } H5E_END_TRY
    if (group_id < 0) {
        H5::PropList pl = create_intermediate_group_property();
        group_id = H5Gcreate(loc.getId(), path.c_str(), pl.getId(), H5P_DEFAULT, H5P_DEFAULT);
    }
    if (group_id < 0) {
        throw error("failed to create group \"" + path + "\"");
    }
    return H5::Group(group_id);
}

inline hid_t open_group(hid_t loc_id, std::string const& path)
{
    hid_t group_id;
    H5E_BEGIN_TRY {
        group_id = H5Gopen(loc_id, path.c_str(), H5P_DEFAULT);
    } H5E_END_TRY
    return group_id;
}

} // namespace h5xx

#endif /* ! H5XX_GROUP_HPP */
