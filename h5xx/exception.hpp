/*
 * Copyright © 2008-2009  Peter Colberg
 * All rights reserved.
 *
 * This file is part of h5xx — a C++ wrapper for the HDF5 library.
 *
 * This software may be modified and distributed under the terms of the
 * 3-clause BSD license.  See accompanying file LICENSE for details.
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

    ~no_autoprint() throw()
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
