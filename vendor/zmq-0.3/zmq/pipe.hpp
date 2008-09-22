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

#ifndef __ZMQ_PIPE_HPP_INCLUDED__
#define __ZMQ_PIPE_HPP_INCLUDED__

#include "i_context.hpp"
#include "i_engine.hpp"
#include "ypipe.hpp"
#include "raw_message.hpp"
#include "config.hpp"

namespace zmq
{
    class pipe_t
    {
    public:

        //  Initialise the pipe.
        pipe_t (struct i_context *source_context_,
            struct i_engine *source_engine_,
            struct i_context *destination_context_,
            struct i_engine *destination_engine_);
        ~pipe_t ();

        //  Write a message to the pipe.
        inline void write (raw_message_t *msg_)
        {
            pipe.write (*msg_);
        }

        //  Write pipe delimiter to the pipe.
        inline void write_delimiter ()
        {
            raw_message_t delimiter;
            raw_message_init_delimiter (&delimiter);
            pipe.write (delimiter);
            flush ();
        }

        //  Flush all the written messages to be accessible for reading.
        inline void flush ()
        {
            if (!pipe.flush ())
                send_revive ();
        }

        //  Returns true, if pipe delimiter was already received.
        bool eop ()
        {
            return endofpipe;
        }

        //  Reads a message from the pipe.
        bool read (raw_message_t *msg);

        //  Make the dead pipe alive once more.
        void revive ();

        //  Notify the other end of the pipe that pipe is to be destroyed.
        void send_destroy_pipe ();

    private:

        void send_revive ();

        //  The message pipe itself
        typedef ypipe_t <raw_message_t, false, message_pipe_granularity>
            underlying_pipe_t;
        underlying_pipe_t pipe;

        //  Identification of the engine sending the messages to the pipe
        i_context *source_context;
        i_engine *source_engine;

        //  Identification of the engine receiving the messages from the pipe
        i_context *destination_context;
        i_engine *destination_engine;

        //  If true we can read messages from the underlying ypipe.
        bool alive; 

        //  True if we've already read the pipe delimiter from
        //  the underlying pipe.
        bool endofpipe;
    }; 

}

#endif
