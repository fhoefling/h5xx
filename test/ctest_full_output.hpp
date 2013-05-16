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

#ifndef CTEST_FULL_OUTPUT_HPP
#define CTEST_FULL_OUTPUT_HPP

#include <boost/test/unit_test_log.hpp>

/**
 * Print CTEST_FULL_OUTPUT to avoid truncation of output by CTest.
 *
 * Requires --log_level=message or --log_level=test_suite.
 *
 * Adding BOOST_GLOBAL_FIXTURE( ctest_full_output ) to the test source triggers
 * the output before any test run.
 */
struct ctest_full_output
{
    ctest_full_output()
    {
        BOOST_TEST_MESSAGE( "Avoid truncation of output by CTest: CTEST_FULL_OUTPUT" );
    }
};

#endif // ! CTEST_FULL_OUTPUT_HPP
