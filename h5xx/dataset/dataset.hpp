/*
 * Copyright © 2008-2011  Peter Colberg and Felix Höfling
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

#ifndef H5XX_DATASET_DATASET_HPP
#define H5XX_DATASET_DATASET_HPP

#include <h5xx/dataset/utility.hpp>

namespace h5xx {


/**
 * Represents an HDF5 dataset.
 *
 */
class dataset
{
public:
    dataset() : hid_(-1) {}

    template <typename h5xxObject>
    dataset(
        h5xxObject const& object,
        std::string const& name
    )
    {
        this->open(object, name);
    }

    ~dataset() { this->close(); }


    /* --- TODO revisit, copied from attribute */
    /**
     * deleted copy constructor
     *
     * Calling the constructor throws an exception. Copying must be elided by
     * return value optimisation. See also "dataset h5xx::move(dataset&)".
     */
    dataset(dataset const& other);

    /**
     * assignment operator
     *
     * Uses the copy-and-swap idiom. Move semantics is achieved in conjunction
     * with "dataset h5xx::move(dataset&)", i.e., the dataset object on the right
     * hand side is empty after move assignment.
     */
    dataset const& operator=(dataset other);

    /** deduce dataspace from dataset */
    operator dataspace() const;
    /* --- TODO revisit, copied from attribute */


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

    /** create dataset at given object (adapted from attribute)
     *  TODO : revisit */
    template <typename h5xxObject> void
    create(
        h5xxObject const& object,
        std::string const& name,
        hid_t type_id,
        dataspace const& space,
        hid_t lcpl_id = H5P_DEFAULT,
        hid_t dcpl_id = H5P_DEFAULT,
        hid_t dapl_id = H5P_DEFAULT
    );

    void write(hid_t type_id, void const* value);

    void read(hid_t type_id, void* buffer);

    template <typename h5xxObject> void
    open(
        h5xxObject const& object,
        std::string const& name
    );

    void close();

    std::string name() const;


private:
    hid_t hid_;
    hid_t type_id_;
    std::string name_;

    template <typename h5xxObject>
    friend void swap(h5xxObject& left, h5xxObject& right);
};


/* --- TODO revisit, copied from attribute */
dataset::dataset(dataset const& other)
  : hid_(other.hid_), type_id_(other.type_id_), name_(other.name_)
{
    // copying would be safe if the exception were disabled.
    throw error("h5xx::dataset can not be copied. Copying must be elided by return value optimisation.");
    H5Iinc_ref(hid_);
}

dataset const& dataset::operator=(dataset other)
{
    swap(*this, other);
    return *this;
}

dataset::operator dataspace() const
{
    H5XX_CHKPT;
    H5XX_PRINT( h5xx::get_name(hid_) );
    hid_t hid = H5Dget_space(hid_);
    H5XX_CHKPT;
    if(hid < 0 ) {
        throw error("dataset +\"" + name() + "\" has invalid dataspace");
    }
    dataspace ds(hid);
    return ds;
}
/* --- TODO revisit, copied from attribute / */


template <typename h5xxObject> void
dataset::create(
    h5xxObject const& object,
    std::string const& name,
    hid_t type_id,
    dataspace const& space,
    hid_t lcpl_id,
    hid_t dcpl_id,
    hid_t dapl_id
)
{
    // hid_t H5Dcreate2( hid_t loc_id, const char *name, hid_t dtype_id, hid_t space_id, hid_t lcpl_id, hid_t dcpl_id, hid_t dapl_id )
    if ((hid_ = H5Dcreate(
                            object.hid(),   // hid_t loc_id IN: Location identifier
                            name.c_str(),   // const char *name IN: Dataset name
                            type_id,        // hid_t dtype_id IN: Datatype identifier
                            space.hid(),    // hid_t space_id IN: Dataspace identifier
                            lcpl_id,        // hid_t lcpl_id IN: Link creation property list
                            dcpl_id,        // hid_t dcpl_id IN: Dataset creation property list
                            dapl_id         // hid_t dapl_id IN: Dataset access property list
                          )
         ) < 0 )
    {
        throw error("creating dataset \"" + name + "\"");
    }
    name_ = name;
    type_id_ = type_id;
}

void dataset::write(hid_t type_id, void const* value)
{
    hid_t mem_space_id, file_space_id, xfer_plist_id;
    mem_space_id = H5S_ALL;
    file_space_id = H5S_ALL;
    xfer_plist_id = H5P_DEFAULT;
//    if (H5Tequal(type_id, type_id_) > 0)
//    {
//        throw error("dataset:type_id mismatch");
//    }
    if (H5Dwrite(hid_, type_id, mem_space_id, file_space_id, xfer_plist_id, value) < 0)
    {
        throw error("writing dataset");
    }
}

void dataset::read(hid_t type_id, void * buffer)
{
    hid_t mem_space_id, file_space_id, xfer_plist_id;
    mem_space_id = H5S_ALL;
    file_space_id = H5S_ALL;
    xfer_plist_id = H5P_DEFAULT;
//    if (H5Tequal(type_id, type_id_) > 0)
//    {
//        throw error("dataset:type_id mismatch");
//    }
    if (H5Dread(hid_, type_id, mem_space_id, file_space_id, xfer_plist_id, buffer) < 0)
    {
        throw error("reading dataset");
    }
}

template <typename h5xxObject> void
dataset::open(
    h5xxObject const& object,
    std::string const& name
)
{
    H5XX_CHKPT;
    if (h5xx::exists_dataset(object, name) > 0) {
        H5XX_CHKPT;
        //hid_t dapl_id = H5Dget_access_plist(object.hid());
        hid_t dapl_id = H5P_DEFAULT;
        if (dapl_id < 0){
            throw error("opening dataset access property list at HDF5 object \"" + get_name(object) + "\"");
        }
        H5XX_CHKPT;
        hid_ = H5Dopen(object.hid(), name.c_str(), dapl_id);
        H5XX_CHKPT;
    }
    if (hid_ < 0){
        throw error("opening dataset \"" + name + "\" at HDF5 object \"" + get_name(object) + "\"");
    }
}

void dataset::close()
{
    if (hid_ >= 0) {
        if(H5Dclose(hid_) < 0){
            throw error("closing h5xx::dataset with ID " + boost::lexical_cast<std::string>(hid_));
        }
        hid_ = -1;
        type_id_ = -1;
        name_.clear();
    }
}

std::string dataset::name() const
{
    return name_;
}

} // namespace h5xx

#endif /* ! H5XX_DATASET_DATASET_HPP */
