/*
 * Copyright © 2013-2014 Felix Höfling
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

#ifndef TEST_FIXTURE_HPP
#define TEST_FIXTURE_HPP

#include <boost/test/unit_test_log.hpp>

#include <h5xx/file.hpp>

//#define NDEBUG
#undef NDEBUG

/**
 * fixture that provides and cleans up an HDF5 file
 */
template <char const* filename>
struct h5file
{
    h5file()
      : file(filename, h5xx::file::trunc)
    {
        BOOST_TEST_MESSAGE("HDF5 file created: " << filename);
    }

    ~h5file()
    {
        file.close(true);
#ifdef NDEBUG
        remove(filename);
        BOOST_TEST_MESSAGE("HDF5 file removed: " << filename);
#endif
    }

    h5xx::file file;
};

#endif // ! TEST_FIXTURE_HPP
