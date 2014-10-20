/*
 * Copyright © 2014 Felix Höfling
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

#ifndef H5XX_POLICY_STRING_HPP
#define H5XX_POLICY_STRING_HPP

#include <h5xx/error.hpp>
#include <h5xx/h5xx.hpp>

namespace h5xx {
namespace policy {
namespace string {

struct null_terminated
{
    enum { is_variable_length = 0 };

    hid_t make_type( size_t size )
    {
       bool err = false;
       hid_t type_id = H5Tcopy(H5T_C_S1);
       err |= H5Tset_size(type_id, size) < 0;
       err |= H5Tset_strpad(type_id, H5T_STR_NULLTERM) < 0;
       if (err) {
           throw error("creating null-terminated string datatype");
       }
       return type_id;
    }

};

struct null_padded
{
    enum { is_variable_length = 0 };

    hid_t make_type( size_t size )
    {
       bool err = false;
       hid_t type_id = H5Tcopy(H5T_C_S1);
       err |= H5Tset_size(type_id, size) < 0;
       err |= H5Tset_strpad(type_id, H5T_STR_NULLPAD) < 0;
       if (err) {
           throw error("creating null-padded string dataspace");
       }
       return type_id;
    }

};

struct space_padded
{
    enum { is_variable_length = 0 };

    hid_t make_type( size_t size )
    {
       bool err = false;
       hid_t type_id = H5Tcopy(H5T_C_S1);
       err |= H5Tset_size(type_id, size) < 0;
       err |= H5Tset_strpad(type_id, H5T_STR_SPACEPAD) < 0;
       if (err) {
           throw error("creating space-padded string dataspace");
       }
       return type_id;
    }

};

struct variable_length
{
    enum { is_variable_length = 1 };

    hid_t make_type( size_t size)
    {
       bool err = false;
       hid_t type_id = H5Tcopy(H5T_C_S1);
//        err |= H5Tset_size(type_id, size) < 0;
       err |= H5Tset_size(type_id, H5T_VARIABLE) < 0;
       if (err) {
           throw error("creating variable-length string dataspace");
       }
       return type_id;
    }
};
} //namespace string
} //namespace policy
} //namespace h5xx

#endif /* ! H5XX_POLICY_STRING_HPP */
