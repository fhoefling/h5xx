/*
 * Copyright © 2013  Felix Höfling
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

//
// the following code snippet has been taken from
// http://stackoverflow.com/questions/15133259/boost-check-no-throw-how-to-get-exception-message-printed
//

#ifndef CATCH_BOOST_NO_THROW_HPP
#define CATCH_BOOST_NO_THROW_HPP

#include <boost/test/unit_test.hpp>

#include <boost/preprocessor/stringize.hpp>
#include <exception>
#include <iostream>

#ifdef BOOST_CHECK_NO_THROW_IMPL
#   undef BOOST_CHECK_NO_THROW_IMPL
#endif

#define BOOST_CHECK_NO_THROW_IMPL( S, TL )                                                      \
    try {                                                                                       \
    S;                                                                                          \
    BOOST_CHECK_IMPL( true, "no exceptions thrown by " BOOST_STRINGIZE( S ), TL, CHECK_MSG ); } \
    catch( const std::exception & e ) {                                                         \
    std::cerr << std::endl                                                                      \
    << "-----------------------------------------------" << std::endl                   \
    << std::endl << "exception message: " << e.what() << std::endl;                 \
    BOOST_CHECK_IMPL( false, "exception thrown by " BOOST_STRINGIZE( S ), TL, CHECK_MSG );      \
    }                                                                                           \
    catch( ... ) {                                                                              \
    std::cerr << std::endl                                                                      \
    << "-----------------------------------------------" << std::endl                   \
    << std::endl << "exception message : <unkown exception>" << std::endl;          \
    BOOST_CHECK_IMPL( false, "exception thrown by " BOOST_STRINGIZE( S ), TL, CHECK_MSG );      \
    }                                                                                           \
    /**/

#define BOOST_WARN_NO_THROW( S )            BOOST_CHECK_NO_THROW_IMPL( S, WARN )
#define BOOST_CHECK_NO_THROW( S )           BOOST_CHECK_NO_THROW_IMPL( S, CHECK )
#define BOOST_REQUIRE_NO_THROW( S )         BOOST_CHECK_NO_THROW_IMPL( S, REQUIRE )

#endif // ! CATCH_BOOST_NO_THROW_HPP
