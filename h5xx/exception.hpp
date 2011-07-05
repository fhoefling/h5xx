/* HDF5 C++ extensions
 *
 * Copyright © 2008-2009  Peter Colberg
 *
 * This file is part of HALMD.
 *
 * HALMD is free software: you can redistribute it and/or modify
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

#ifndef H5XX_EXCEPTION_HPP
#define H5XX_EXCEPTION_HPP

#include <h5xx/hdf5_compat.hpp>

namespace h5xx {

template <typename Exception>
class no_autoprint : public H5::Exception
{
public:
    no_autoprint()
    {
        H5::Exception::getAutoPrint(func, &client_data);
        H5::Exception::dontPrint();
    }

    ~no_autoprint()
    {
        H5::Exception::setAutoPrint(func, client_data);
    }

private:
    H5E_auto_t func;
    void* client_data;
};

#define H5XX_NO_AUTO_PRINT(exception) h5xx::no_autoprint<exception> __no_autoprint;

} // namespace h5xx

#endif /* ! H5XX_EXCEPTION_HPP */
