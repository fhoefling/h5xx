/*
 * Copyright © 2013 Manuel Dibak
 * Copyright © 2013 Felix Höfling
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


#ifndef H5XX_FILE_HPP
#define H5XX_FILE_HPP

#include <h5xx/hdf5_compat.hpp>
#include <h5xx/error.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>
#include <string>
#include <vector>

namespace h5xx {

htri_t is_hdf5_file(std::string const& filename)
{
    H5E_BEGIN_TRY {
        return H5Fis_hdf5(filename.c_str());
    } H5E_END_TRY
}

class file : boost::noncopyable
{
public:
    enum mode
    {
        in = 0x0000u        /* H5F_ACC_RDONLY absence of rdwr => rd-only */
      , out = 0x0001u       /* H5F_ACC_RDWR open for read and write    */
      , trunc = 0x0002u     /* H5F_ACC_TRUNC overwrite existing files   */
      , excl = 0x0004u      /* H5F_ACC_EXCL fail if file already exists*/
    };

    /** default constructor */
    file() : hid_(-1) {}

    /** open file upon construction */
    explicit file(std::string const& filename, unsigned mode = in | out);

    /** close file upon destruction */
    ~file();

    /** return HDF5 object ID */
    hid_t hid() const
    {
        return hid_;
    }

    /** open HDF5 file in specified mode */
    void open(std::string const& filename, unsigned mode = in | out);

    /** close HDF5 file */
    void close();

    /** flush all buffers associated with the file to disk */
    void flush() const;

    /** return filename on disk */
    std::string name() const;

private:
    /** HDF5 object ID */
    hid_t hid_;
};

file::file(std::string const& filename, unsigned mode)
  : hid_(-1)
{
    open(filename, mode);
}

file::~file()
{
    close();
}

void file::open(std::string const& filename, unsigned mode)
{
    // check that object is not yet in use
    if (hid_ >= 0) {
        throw error("h5xx::file object is already open");
    }

    // check for invalid combination of opening flags
    if (((mode & trunc) && (mode & excl)) || ((mode & (trunc | excl)) && !(mode & out))) {
        throw error("h5xx::file: invalid opening mode: " + boost::lexical_cast<std::string>(mode));
    }

    htri_t is_hdf5 = is_hdf5_file(filename);
    if (is_hdf5 >= 0) { // file exists and may be valid HDF5
        if (mode & out) {
            if (mode & excl) {
                throw error("refuse to overwrite existing HDF5 file: " + filename);
            }
            if (mode & trunc) { // truncate file
                hid_ = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
            }
            else { // append to file
                hid_ = H5Fopen(filename.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
            }
        }
        else { // read-only
            if (is_hdf5 == 0) {
                throw error("not a valid HDF5 file: " + filename);
            }
            hid_ = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
        }
    }
    else { // file does not exist, or other error
        if(mode & out) { // create non-existing file
            hid_ = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
        }
        else {
            throw error("read-only access to non-existing HDF5 file: " + filename);
        }
    }
}

void file::flush() const
{
    if (hid_ < 0) {
        return;
    }
    if (H5Fflush(hid_, H5F_SCOPE_LOCAL) < 0) {
        throw error("flushing HDF5 file: " + name());
    }
}

void file::close()
{
    if (hid_ < 0) {
        return;
    }
    if (H5Fclose(hid_) < 0) {
        throw error("closing HDF5 file: " + name() +
                    ", file ID: " + boost::lexical_cast<std::string>(hid_));
    }
    hid_ = -1;
}

std::string file::name() const
{
    if (hid_ < 0) {
        throw error("no HDF5 file associated to h5xx::file object");
    }
    ssize_t size = H5Fget_name(hid_, NULL, 0);        // determine string length
    if (size < 0) {
        throw error("retrieving name of HDF5 file with ID " + boost::lexical_cast<std::string>(hid_));
    }
    std::vector<char> buffer(size + 1);
    H5Fget_name(hid_, &*buffer.begin(), buffer.size()); // get string data
    return &*buffer.begin();
}

} // namespace h5xx

#endif // ! H5XX_FILE_HPP
