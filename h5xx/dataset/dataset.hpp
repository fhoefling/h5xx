/*
 * Copyright © 2008-2011  Peter Colberg and Felix Höfling
 * Copyright © 2014       Klaus Reuter
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
#include <h5xx/dataspace.hpp>
#include <h5xx/datatype.hpp>
#include <h5xx/policy.hpp>

namespace h5xx {

/**
 * Template class to wrap the HDF5 dataset.
 */
class dataset
{
public:
    dataset() : hid_(-1) {}

    /** open existing dataset */
    template <typename h5xxObject>
    dataset(h5xxObject const& object, std::string const& name, hid_t dapl_id = H5P_DEFAULT);

    /** create a new dataset */
    template <typename h5xxObject>
    dataset(h5xxObject const& object, std::string const& name, datatype const& dtype, dataspace const& dspace,
               h5xx::policy::dataset_creation_property_list dcpl = h5xx::policy::default_dataset_creation_property_list);

    /** create a new dataset, raw hdf5 property lists are accepted */
    template <typename h5xxObject>
    dataset(h5xxObject const& object, std::string const& name, datatype const& dtype, dataspace const& dspace,
        hid_t lcpl_id = H5P_DEFAULT, hid_t dcpl_id = H5P_DEFAULT, hid_t dapl_id = H5P_DEFAULT);

    /** destructor, implicitly closes the dataset's hid_ */
    ~dataset();

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

    /** return HDF5 object ID */
    hid_t hid() const;

    /** returns true if associated to a valid HDF5 object */
    bool valid() const;

    /** create dataset at the given object */
    template <typename h5xxObject> void
    create(h5xxObject const& object, std::string const& name, datatype const& type, dataspace const& space,
        hid_t lcpl_id = H5P_DEFAULT, hid_t dcpl_id = H5P_DEFAULT, hid_t dapl_id = H5P_DEFAULT);

    /** write value to the dataset */
    void write(hid_t type_id, void const* value, hid_t mem_space_id = H5S_ALL, hid_t file_space_id = H5S_ALL, hid_t xfer_plist_id = H5P_DEFAULT);

    /** read from the dataset into the buffer */
    void read(hid_t type_id, void* buffer, hid_t mem_space_id = H5S_ALL, hid_t file_space_id = H5S_ALL, hid_t xfer_plist_id = H5P_DEFAULT);

    /** get the name of the dataset */
    std::string name() const;

    /** return copy of dataset's type */
    hid_t get_type();

private:
    /** HDF5 handle of the dataset */
    hid_t hid_;

    template <typename h5xxObject>
    friend void swap(h5xxObject& left, h5xxObject& right);
};

template <typename h5xxObject>
dataset::dataset(h5xxObject const& object, std::string const& name, hid_t dapl_id)
  : hid_(-1)
{
    if (h5xx::exists_dataset(object, name))
    {
        hid_ = H5Dopen(object.hid(), name.c_str(), dapl_id);
    }
    if (hid_ < 0)
    {
        throw error("opening dataset \"" + name + "\" at HDF5 object \"" + get_name(object) + "\"");
    }
}

template <typename h5xxObject>
dataset::dataset(h5xxObject const& object, std::string const& name, datatype const& dtype, dataspace const& dspace,
               h5xx::policy::dataset_creation_property_list dcpl)
{
    hid_ = -1;
    hid_t lcpl_id = H5P_DEFAULT;
    hid_t dcpl_id = H5P_DEFAULT;
    hid_t dapl_id = H5P_DEFAULT;
    dcpl_id = dcpl.get();
    this->create(object, name, dtype, dspace, lcpl_id, dcpl_id, dapl_id);
}

template <typename h5xxObject>
dataset::dataset(h5xxObject const& object, std::string const& name, datatype const& dtype, dataspace const& dspace,
    hid_t lcpl_id, hid_t dcpl_id, hid_t dapl_id)
{
    hid_ = -1;
    this->create(object, name, dtype, dspace, lcpl_id, dcpl_id, dapl_id);
}

dataset::~dataset()
{
    if (hid_ >= 0) {
        if(H5Dclose(hid_) < 0){
            throw error("closing h5xx::dataset with ID " + boost::lexical_cast<std::string>(hid_));
        }
        hid_ = -1;
    }
}

dataset::dataset(dataset const& other)
  : hid_(other.hid_)
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
    hid_t hid = H5Dget_space(hid_);
    if(hid < 0 ) {
        throw error("dataset +\"" + name() + "\" has invalid dataspace");
    }
    dataspace ds(hid);
    return ds;
}

template <typename h5xxObject> void
dataset::create( h5xxObject const& object, std::string const& name, datatype const& type, dataspace const& space,
    hid_t lcpl_id, hid_t dcpl_id, hid_t dapl_id
)
{
    if (h5xx::exists_dataset(object, name))
    {
        throw error("dataset \"" + name + "\" already exists");
    }
    else
    {
        if ((hid_ = H5Dcreate(
                                object.hid(),       // hid_t loc_id IN: Location identifier
                                name.c_str(),       // const char *name IN: Dataset name
                                type.get_type_id(), // hid_t dtype_id IN: Datatype identifier
                                space.hid(),        // hid_t space_id IN: Dataspace identifier
                                lcpl_id,            // hid_t lcpl_id IN: Link creation property list
                                dcpl_id,            // hid_t dcpl_id IN: Dataset creation property list
                                dapl_id             // hid_t dapl_id IN: Dataset access property list
                              )
             ) < 0 )
        {
            throw error("creating dataset \"" + name + "\"");
        }
    }
}

void dataset::write(hid_t type_id, void const* value, hid_t mem_space_id, hid_t file_space_id, hid_t xfer_plist_id)
{
    if (H5Dwrite(hid_, type_id, mem_space_id, file_space_id, xfer_plist_id, value) < 0)
    {
        throw error("writing dataset");
    }
}

void dataset::read(hid_t type_id, void * buffer, hid_t mem_space_id, hid_t file_space_id, hid_t xfer_plist_id)
{
    if (H5Dread(hid_, type_id, mem_space_id, file_space_id, xfer_plist_id, buffer) < 0)
    {
        throw error("reading dataset");
    }
}

hid_t dataset::get_type()
{
    hid_t type_id = H5Dget_type(hid_);
    if (type_id < 0)
    {
        throw error("failed to obtain type_id of dataset \"" + name() + "\"");
    }
    return type_id;
}

std::string dataset::name() const
{
    return get_name(hid_);
}

hid_t dataset::hid() const
{
    return hid_;
}

bool dataset::valid() const
{
    return (hid_ >= 0);
}
// --- END dataset class method implementations ---


/**
 * free function to create datasets
 */
template <typename h5xxObject>
void create_dataset(h5xxObject const& object, std::string const& name, datatype const& dtype, dataspace const& dspace,
               h5xx::policy::dataset_creation_property_list dcpl = h5xx::policy::default_dataset_creation_property_list)
{
    hid_t lcpl_id = H5P_DEFAULT;
    hid_t dcpl_id = H5P_DEFAULT;
    hid_t dapl_id = H5P_DEFAULT;
    dcpl_id = dcpl.get();
    dataset data_set(object, name, dtype, dspace, lcpl_id, dcpl_id, dapl_id);
}

} // namespace h5xx

#endif /* ! H5XX_DATASET_DATASET_HPP */
