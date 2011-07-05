/*
 * Copyright © 2008-2010  Peter Colberg and Felix Höfling
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

#ifndef H5XX_CTYPE_HPP
#define H5XX_CTYPE_HPP

#include <h5xx/hdf5_compat.hpp>

namespace h5xx {
namespace detail {

/*
 * Translate C/C++ type to HDF5 native data type.
 */
template <typename T>
struct ctype;

#define H5XX_MAKE_CTYPE(T, H5T)         \
    template <>                         \
    struct ctype<T>                     \
    {                                   \
        static hid_t hid()              \
        {                               \
            return H5Tcopy(H5T);        \
        }                               \
    }

H5XX_MAKE_CTYPE( char,                  H5T_NATIVE_CHAR );
H5XX_MAKE_CTYPE( signed char,           H5T_NATIVE_SCHAR );
H5XX_MAKE_CTYPE( unsigned char,         H5T_NATIVE_UCHAR );
H5XX_MAKE_CTYPE( short,                 H5T_NATIVE_SHORT );
H5XX_MAKE_CTYPE( unsigned short,        H5T_NATIVE_USHORT );
H5XX_MAKE_CTYPE( int,                   H5T_NATIVE_INT );
H5XX_MAKE_CTYPE( unsigned int,          H5T_NATIVE_UINT );
H5XX_MAKE_CTYPE( long,                  H5T_NATIVE_LONG );
H5XX_MAKE_CTYPE( unsigned long,         H5T_NATIVE_ULONG );
H5XX_MAKE_CTYPE( long long,             H5T_NATIVE_LLONG );
H5XX_MAKE_CTYPE( unsigned long long,    H5T_NATIVE_ULLONG );
H5XX_MAKE_CTYPE( float,                 H5T_NATIVE_FLOAT );
H5XX_MAKE_CTYPE( double,                H5T_NATIVE_DOUBLE );
H5XX_MAKE_CTYPE( long double,           H5T_NATIVE_LDOUBLE );
H5XX_MAKE_CTYPE( bool,                  H5T_NATIVE_HBOOL );

#undef H5XX_MAKE_CTYPE

} // namespace detail
} // namespace h5xx

#endif /* ! H5XX_CTYPE_HPP */
