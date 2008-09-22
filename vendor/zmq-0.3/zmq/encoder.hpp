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

#ifndef __ZMQ_ENCODER_HPP_INCLUDED__
#define __ZMQ_ENCODER_HPP_INCLUDED__

#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <algorithm>

namespace zmq
{

    //  Helper base class for encoders. It implements the state machine that
    //  fills the outgoing buffer. Derived classes should implement individual
    //  state machine actions.

    template <typename T> class encoder_t
    {
    public:

        inline encoder_t ()
        {
        }

        //  The function tries to fill the supplied chunk by binary data.
        //  Returns the size of data actually filled in.
        inline size_t read (unsigned char *data_, size_t size_)
        {
            size_t pos = 0;

            while (true) {
                if (to_write) {
                    size_t to_copy = std::min (to_write, size_ - pos);
                    memcpy (data_ + pos, write_pos, to_copy);
                    pos += to_copy;
                    write_pos += to_copy;
                    to_write -= to_copy;
                }
                else if (!((static_cast <T*> (this)->*next) ()))
                    return pos;
                if (pos == size_)
                    return pos;
            }
        }

    protected:

        typedef bool (T::*step_t) ();

        //  This function should be called from derived class to write the data
        //  to the buffer and schedule next state machine action
        inline void next_step (void *write_pos_, size_t to_write_,
            step_t next_)
        {
            write_pos = (unsigned char*) write_pos_;
            to_write = to_write_;
            next = next_;
        }

    private:

        unsigned char *write_pos;
        size_t to_write;
        step_t next;
    };

}

#endif
