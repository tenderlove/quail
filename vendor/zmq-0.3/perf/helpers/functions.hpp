/*
    Copyright (c) 2007-2008 FastMQ Inc.

    This file is part of 0MQ.

    0MQ is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    0MQ is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __PERF_FUNCTIONS_HPP_INCLUDED__
#define __PERF_FUNCTIONS_HPP_INCLUDED__

#include <sstream>

namespace perf
{

    //  Converts value supplied into the string.
    template <typename T> inline std::string to_string (const T &input_)
    {
        std::stringstream string_stream;
        string_stream << input_;
        return string_stream.str ();
    }
}
#endif
