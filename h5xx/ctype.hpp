/* HDF5 C++ extensions
 *
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

#include <h5xx/hdf5.hpp>

namespace H5
{

/*
 * fundamental type to HDF5 native data type translation
 */
template <typename T>
struct ctype;

#define MAKE_CTYPE(CTYPE, H5TYPE) \
template <> struct ctype<CTYPE> \
{ operator H5::PredType const& () { return H5::PredType::NATIVE_##H5TYPE;} }

MAKE_CTYPE(float, FLOAT);
MAKE_CTYPE(double, DOUBLE);
MAKE_CTYPE(long double, LDOUBLE);
MAKE_CTYPE(int8_t, INT8);
MAKE_CTYPE(uint8_t, UINT8);
MAKE_CTYPE(int16_t, INT16);
MAKE_CTYPE(uint16_t, UINT16);
MAKE_CTYPE(int32_t, INT32);
MAKE_CTYPE(uint32_t, UINT32);
MAKE_CTYPE(int64_t, INT64);
MAKE_CTYPE(uint64_t, UINT64);

#undef MAKE_CTYPE

} // namespace H5

#endif /* ! H5XX_CTYPE_HPP */
