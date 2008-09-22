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

#ifndef __ZMQ_I_SIGNALER_HPP_INCLUDED__
#define __ZMQ_I_SIGNALER_HPP_INCLUDED__

namespace zmq
{
    //  Virtual interface used to send signals. Individual implementations
    //  may restrict the number of possible signal types to send.
    struct i_signaler
    {
        //  The destructor shouldn't be virtual, however, not defining it as
        //  such results in compiler warnings with some compilers.
        virtual ~i_signaler () {};

        //  Send a signal with a specific ID.
        virtual void signal (int signal_) = 0;
    };

}

#endif
