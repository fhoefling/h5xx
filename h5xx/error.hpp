/*
 * Copyright © 2010  Peter Colberg
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

#ifndef H5XX_ERROR_HPP
#define H5XX_ERROR_HPP

#include <stdexcept>

namespace h5xx {

/**
 * h5xx wrapper error
 */
class error
  : virtual public std::exception
{
public:
    error(std::string const& desc)
        : desc_(desc) {}

    virtual ~error() throw() {}

    char const* what() const throw()
    {
        return desc_.c_str();
    }

private:
    std::string desc_;
};

} // namespace h5xx

#endif /* ! H5XX_ERROR_HPP */
