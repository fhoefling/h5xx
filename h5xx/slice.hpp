/*
 * Copyright © 2014-2015 Felix Höfling
 * Copyright © 2014-2015 Klaus Reuter
 * All rights reserved.
 *
 * This file is part of h5xx — a C++ wrapper for the HDF5 library.
 *
 * This software may be modified and distributed under the terms of the
 * 3-clause BSD license.  See accompanying file LICENSE for details.
 */

#ifndef H5XX_SLICE_HPP
#define H5XX_SLICE_HPP

#include <algorithm>
#include <iterator>
#include <vector>
#include <string>
#include <sstream>

// --- C regular expression support
#include <sys/types.h>
#include <regex.h>
#include <assert.h>


namespace h5xx {


class slice {
public:
    /**
     * Slice constructor accepting a string with a numpy-like slicing notation.
     */
    slice(const std::string & slice_str);

//   --- constructor with zero offset by default?
//    template <class ArrayType>
//    slice(ArrayType count);

    template <class ArrayType>
    slice(ArrayType offset, ArrayType count);

    template <class ArrayType>
    slice(ArrayType offset, ArrayType count, ArrayType stride);

    template <class ArrayType>
    slice(ArrayType offset, ArrayType count, ArrayType stride, ArrayType block);

    int rank() const;

    const std::vector<hsize_t> & get_count() const;
    const std::vector<hsize_t> & get_offset() const;
    const std::vector<hsize_t> & get_stride() const;
    const std::vector<hsize_t> & get_block() const;

    /**
     * Return true if a slice string has been set via the constructor.
     */
    bool has_string() const;

    /**
     * Fill offset_, count_, stride_, block_ arrays based on the slice string.
     * When ranges ":" are used, the extents of the dataset must be given.
     */
    void parse_string(const std::vector<hsize_t> & extents = std::vector<hsize_t>());

private:
    std::vector<hsize_t> offset_, count_, stride_, block_;

    std::string slice_str_;

    // --- regular expression stuff, used inside the parse_string() method
    regex_t slicing_fmt_rex;               // "1:2:3,4,:"
    regex_t colon_rex;                     // ":"
    regex_t slice_single_rex;              // "1"
    regex_t slice_range_rex;               // "1:4"
    regex_t slice_range_stride_rex;        // "1:4:2"
    regex_t slice_full_range_rex;          // ":"
    regex_t slice_full_range_ceil_rex;     // ":2"
    regex_t slice_full_range_floor_rex;    // "2:"
    regex_t slice_full_range_stride_rex;   // "::2"
    void prepare_rex(regex_t & rex, std::string const& str);
    bool match_rex(regex_t const& rex, std::string const& str);
};


inline slice::slice(const std::string & slice_str)
{
    slice_str_ = slice_str;
}


//template <class ArrayType>
//slice::slice(ArrayType count)
//{
//    std::copy(count.begin(), count.end(), std::back_inserter(count_));
//    offset_.assign(count.size(), 0);
//    stride_.clear();
//    block_.clear();
//}

template <class ArrayType>
slice::slice(ArrayType offset, ArrayType count)
{
    if (offset.size() != count.size())
    {
        throw error("slice specification arrays must have identical length");
    }
    std::copy(offset.begin(), offset.end(), std::back_inserter(offset_));
    std::copy(count.begin(),  count.end(),  std::back_inserter(count_));
    stride_.clear();
    block_.clear();
}

template <class ArrayType>
slice::slice(ArrayType offset, ArrayType count, ArrayType stride)
{
    if ((offset.size() != count.size()) || (count.size() != stride.size()))
    {
        throw error("slice specification arrays must have identical length");
    }
    std::copy(offset.begin(), offset.end(), std::back_inserter(offset_));
    std::copy(count.begin(),  count.end(),  std::back_inserter(count_));
    std::copy(stride.begin(), stride.end(), std::back_inserter(stride_));
    block_.clear();
}

template <class ArrayType>
slice::slice(ArrayType offset, ArrayType count, ArrayType stride, ArrayType block)
{
    if ((offset.size() != count.size()) || (count.size() != stride.size()) || (stride.size() != block.size()))
    {
        throw error("slice specification arrays must have identical length");
    }
    std::copy(offset.begin(), offset.end(), std::back_inserter(offset_));
    std::copy(count.begin(),  count.end(),  std::back_inserter(count_));
    std::copy(stride.begin(), stride.end(), std::back_inserter(stride_));
    std::copy(block.begin(),  block.end(),  std::back_inserter(block_));
}



inline int slice::rank() const
{
    return count_.size();
}

inline const std::vector<hsize_t> & slice::get_offset() const
{
    return offset_;
}

inline const std::vector<hsize_t> & slice::get_count() const
{
    return count_;
}

inline const std::vector<hsize_t> & slice::get_stride() const
{
    return stride_;
}

inline const std::vector<hsize_t> & slice::get_block() const
{
    return block_;
}



inline bool slice::has_string() const
{
    return (slice_str_.length() > 0);
}

inline void slice::parse_string(const std::vector<hsize_t> & extents)
{
    if (!has_string()) {
        throw error( "missing array slice string" );
    }

    // --- initialize regular expression patterns
    prepare_rex(slicing_fmt_rex,             "^([0-9:]+,)*[0-9:]+$");   // TODO make exact
    prepare_rex(colon_rex,                   ":");
    prepare_rex(slice_single_rex,            "^[0-9]+$");
    prepare_rex(slice_range_rex,             "^[0-9]+:[0-9]+$");
    prepare_rex(slice_range_stride_rex,      "^[0-9]+:[0-9]+:[0-9]+$");
    prepare_rex(slice_full_range_rex,        "^:$");
    prepare_rex(slice_full_range_ceil_rex,   "^:[0-9]+$");
    prepare_rex(slice_full_range_floor_rex,  "^[0-9]+:$");
    prepare_rex(slice_full_range_stride_rex, "^::[0-9]+$");

    // --- check if slice_str_ contains a valid slicing notation
    if (!match_rex(slicing_fmt_rex, slice_str_)) {
        throw error( std::string("array slicing format is invalid : ").append(slice_str_) );
    }

    // --- create string vector with slice specification separated for each dimension
    std::vector<std::string> slice_specs = chop(slice_str_, ",");

    // --- in case of range operations ":", the extent vector must be provided and has to fit the string dimension
    if (match_rex(colon_rex, slice_str_))
        if ((extents.size() == 0) || (slice_specs.size() != extents.size()))
            throw error("dimensions of slice string and dataset extents do not match");

    // --- decrypt the slice specification, dimension by dimension
    typedef std::vector<std::string>::iterator vsit_t;
    int dim = 0;
    for (vsit_t it = slice_specs.begin(); it < slice_specs.end(); ++it)
    {
        std::string spec = *it;

        if      ( match_rex(slice_single_rex, spec) )
        { // "a"
            int i = str2num<int>(spec);
            offset_.push_back(i);
            count_.push_back(1);
            stride_.push_back(1);
        }
        else if ( match_rex(slice_range_rex, spec) )
        { // "a:b"
            std::vector<std::string> range_nums = chop(spec, ":");
            int lo = str2num<int>( range_nums[0] );
            int hi = str2num<int>( range_nums[1] );
            offset_.push_back(lo);
            count_.push_back(hi-lo);
            stride_.push_back(1);
        }
        else if ( match_rex(slice_range_stride_rex, spec) )
        { // "a:b:s"
            std::vector<std::string> range_nums = chop(spec, ":");
            int lo = str2num<int>( range_nums[0] );
            int hi = str2num<int>( range_nums[1] );
            int dx = str2num<int>( range_nums[2] );
            offset_.push_back(lo);
            count_.push_back( (hi-lo-1)/dx +1 );
            stride_.push_back(dx);
        }
        else if ( match_rex(slice_full_range_rex, spec) )
        { // ":"
            offset_.push_back(0);
            count_.push_back(extents[dim]);
            stride_.push_back(1);
        }
        else if ( match_rex(slice_full_range_ceil_rex, spec) )
        { // ":b"
            std::vector<std::string> range_nums = chop(spec, ":");
            int hi = str2num<int>( range_nums[0] );
            offset_.push_back(0);
            count_.push_back(hi);
            stride_.push_back(1);
        }
        else if ( match_rex(slice_full_range_floor_rex, spec) )
        { // "a:"
            std::vector<std::string> range_nums = chop(spec, ":");
            int lo = str2num<int>( range_nums[0] );
            offset_.push_back(lo);
            count_.push_back(extents[dim]-lo);
            stride_.push_back(1);
        }
        else if ( match_rex(slice_full_range_stride_rex, spec) )
        { // "::s"
            std::vector<std::string> range_nums = chop(spec, ":");
            int dx = str2num<int>( range_nums[0] );
            offset_.push_back(0);
            count_.push_back((extents[dim]-1)/dx + 1);
            stride_.push_back(dx);
        }
        else
        {
            throw error( std::string("invalid slice specification : ").append(spec) );
        }
        ++dim;
    }

    // potential memory leak in case of thrown exceptions
    {
        regfree(&slicing_fmt_rex);
        regfree(&colon_rex);
        regfree(&slice_single_rex);
        regfree(&slice_range_rex);
        regfree(&slice_range_stride_rex);
        regfree(&slice_full_range_rex);
        regfree(&slice_full_range_ceil_rex);
        regfree(&slice_full_range_floor_rex);
        regfree(&slice_full_range_stride_rex);
    }
}


/**
 * compile regular expression handle. with error checking
 */
inline void slice::prepare_rex(regex_t & rex, std::string const& str)
{
    int ret = regcomp(&rex, str.c_str(), REG_EXTENDED);
    if (ret != 0) {
        const int errbuf_size = 256;
        char errbuf[errbuf_size];
        regerror(ret, &rex, errbuf, errbuf_size);
        throw error( std::string("regex compilation : ").append(errbuf) );
    }
}

/**
 * check if a string matches to a regular expression
 */
inline bool slice::match_rex(regex_t const& rex, std::string const& str)
{
    int ret = regexec(&rex, str.c_str(), 0, NULL, 0);
    return (ret == 0);
}

} // namespace h5xx


#endif // ! H5XX_SLICE_HPP
