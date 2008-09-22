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

#ifndef __ZMQ_MESSAGE_HPP_INCLUDED__
#define __ZMQ_MESSAGE_HPP_INCLUDED__

#include "raw_message.hpp"

namespace zmq
{

    //  0MQ message. Don't change the body of the message once you've
    //  copied it - the behaviour would be undefined. Don't change the body
    //  of the message received.

    class message_t : private raw_message_t
    {
    public:

        //  Creates empty message (0 bytes long).
        inline message_t ()
        {
            content = (message_content_t*) raw_message_t::vsm_tag;
            vsm_size = 0;
        }

        //  Creates message size_ bytes long.
        inline message_t (size_t size_)
        {
            raw_message_init (this, size_);
        }

        //  Creates message from the supplied buffer. 0MQ takes care of
        //  deallocating the buffer once it is not needed. The deallocation
        //  function is supplied in ffn_ parameter. If ffn_ is NULL, no
        //  deallocation happens - this is useful for sending static buffers.
        inline message_t (void *data_, size_t size_, free_fn *ffn_)
        {
            raw_message_init (this, data_, size_, ffn_);
        }

        //  Destroys the message.
        inline ~message_t ()
        {
            raw_message_destroy (this);
        }

        //  Destroys old content of the message and allocates buffer for the
        //  new message body. Having this as a separate function allows user
        //  to reuse once-allocated message for multiple times.
        inline void rebuild (size_t size_)
        {
            raw_message_destroy (this);
            raw_message_init (this, size_);            
        }

        //  Same as above, however, the message is rebuilt from the supplied
        //  buffer. See appropriate constructor for discussion of buffer
        //  deallocation mechanism.
        inline void rebuild (void *data_, size_t size_, free_fn *ffn_)
        {
            raw_message_destroy (this);
            raw_message_init (this, data_, size_, ffn_);            
        }

        //  Moves the message content from one message to the another. If the
        //  destination message have contained data prior to the operation
        //  these get deallocated. The source message will contain 0 bytes
        //  of data after the operation.
        inline void move_to (message_t *msg_)
        {
            raw_message_move (this, (raw_message_t*) msg_);
        }

        //  Copies the message content from one message to the another. If the
        //  destination message have contained data prior to the operation
        //  these get deallocated.
        inline void copy_to (message_t *msg_)
        {
            raw_message_copy (this, (raw_message_t*) msg_);
        }

        //  Returns pointer to message's data buffer.
        inline void *data ()
        {
            return raw_message_data (this);
        }

        //  Returns the size of message data buffer.
        inline size_t size ()
        {
            return raw_message_size (this);
        }

    private:

        //  Disable implicit message copying, so that users won't use shared
        //  messages (less efficient) without being aware of the fact.
        message_t (const message_t&);
    };

}

#endif
