/*
 * Copyright © 2014 Felix Höfling
 * Copyright © 2014 Klaus Reuter
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

#ifndef H5XX_POLICY_POLICY_HPP
#define H5XX_POLICY_POLICY_HPP

//#include <vector>
//#include <memory>

namespace h5xx {
namespace policy {

class dataset_creation_property
{
public:
    virtual ~dataset_creation_property() {}
    virtual void set(hid_t plist) const = 0;
};

class dataset_creation_property_list
{
public:
    dataset_creation_property_list()
    {
        dcpl_ = H5Pcreate (H5P_DATASET_CREATE);
    }

    dataset_creation_property_list(dataset_creation_property_list& src)
    {
        if ((dcpl_ = H5Pcopy(src.get())) < 0)
        {
            throw error("dataset_creation_property_list, copy constructor");
        }
    }

    dataset_creation_property_list &operator=(dataset_creation_property_list& src)
    {
        if ((dcpl_ = H5Pcopy(src.get())) < 0)
        {
            throw error("dataset_creation_property_list, assignment operator");
        }
        return *this;
    }

    ~dataset_creation_property_list()
    {
        H5Pclose(dcpl_);
    }

    void add(h5xx::policy::dataset_creation_property const& prop)
    {
        prop.set(dcpl_);
    }

    hid_t get() {
        return dcpl_;
    };

private:
    hid_t dcpl_;
};

static dataset_creation_property_list default_dataset_creation_property_list;

} //namespace policy
} //namespace h5xx

#endif // ! H5XX_POLICY_POLICY_HPP
