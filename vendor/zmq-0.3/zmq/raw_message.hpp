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

#ifndef __ZMQ_RAW_MESSAGE_HPP_INCLUDED__
#define __ZMQ_RAW_MESSAGE_HPP_INCLUDED__

#include <assert.h>

#include "stdint.hpp"
#include "config.hpp"
#include "atomic_counter.hpp"

namespace zmq
{

    //  Prototype for the message body deallocation functions.
    //  It is deliberately defined in the way to comply with standard C free.
    typedef void (free_fn) (void *data_);

    //  Shared message buffer. Message data are either allocated in one block
    //  with this structure - thus avoiding one malloc/free pair - or they are
    //  stored in used-supplied memory. In the latter case, ffn member stores
    //  pointer to the function to be used to deallocate the data.
    //  If the buffer is actually shared (there are at least 2 references to it)
    //  refcount member contains number of references.

    struct message_content_t
    {
        void *data;
        size_t size;
        free_fn *ffn;
        atomic_counter_t refcount;
    };

    //  POD version of 0MQ message. The point is to avoid any implicit
    //  construction/destruction/copying of the structure.
    //  From user's point of view raw message is wrapped in message_t
    //  class which makes its usage more convenient.
    //  If 'shared' is true, message content pointed to by 'content' is shared,
    //  i.e. you have to use reference counting to manage its lifetime
    //  rather than straighforward malloc/free.

    struct raw_message_t
    {
        enum {
            delimiter_tag = 0,
            vsm_tag = 1
        };

        message_content_t *content;
        bool shared;
        uint16_t vsm_size;
        unsigned char vsm_data [max_vsm_size];
    };

    //  Initialises a message of the specified size.
    inline void raw_message_init (raw_message_t *msg_, size_t size_)
    {
        if (size_ <= max_vsm_size) {
            msg_->content = (message_content_t*) raw_message_t::vsm_tag;
            msg_->vsm_size = size_;
        }
        else {
            msg_->shared = false;
            msg_->content = (message_content_t*) malloc (
                sizeof (message_content_t) + size_);
            assert (msg_->content);
            msg_->content->data = (void*) (msg_->content + 1);
            msg_->content->size = size_;
            msg_->content->ffn = NULL;
        }

    }

    //  Creates a message from the supplied buffer. From this point on
    //  message takes care of buffer's lifetime - it will deallocate the buffer
    //  once it is not needed. This functionality is useful to avoid copying
    //  data in case you are dealing with legacy code.
    //  In other cases, however, standard initialisation should be prefered
    //  as it is more efficient when compared to this one.
    inline void raw_message_init (raw_message_t *msg_,
        void *data_, size_t size_, free_fn *ffn_)
    {
        msg_->shared = false;
        msg_->content = (message_content_t*) malloc (
            sizeof (message_content_t) + size_);
        assert (msg_->content);
        msg_->content->data = data_;
        msg_->content->size = size_;
        msg_->content->ffn = ffn_;
    }

    //  Initialises raw_message_t to be a pipe delimiter.
    inline void raw_message_init_delimiter (raw_message_t *msg_)
    {
        msg_->content = (message_content_t*) raw_message_t::delimiter_tag;
    }

    //  Releases the resources associated with the message. Obviously, if
    //  message content is shared, it releases one reference only and destroys
    //  the content only if there is no reference left.
    inline void raw_message_destroy (raw_message_t *msg_)
    {
        //  For VSMs and delimiters there are no resources to free
        if (msg_->content ==
              (message_content_t*) raw_message_t::delimiter_tag ||
              msg_->content == (message_content_t*) raw_message_t::vsm_tag)
            return;

        //  If the content is not shared, or if it is shared and the reference
        //  count has dropped to zero, deallocate it.
        if (!msg_->shared || !msg_->content->refcount.sub (1)) {
            if (msg_->content->ffn)
                msg_->content->ffn (msg_->content->data);
            free (msg_->content);
        }
    }

    //  Moves the message content from one message to the another. If the
    //  destination message have contained data prior to the operation
    //  these get deallocated. The source message will contain 0 bytes of data
    //  after the operation.
    inline void raw_message_move (raw_message_t *src_, raw_message_t *dest_)
    {
        raw_message_destroy (dest_);
        *dest_ = *src_;
        raw_message_init (src_, 0);
    }

    //  Copies the message content from one message to the another. If the
    //  destination message have contained data prior to the operation
    //  these get deallocated.
    inline void raw_message_copy (raw_message_t *src_, raw_message_t *dest_)
    {
        raw_message_destroy (dest_);

        //  VSMs and delimiters require no special handling.
        if (src_->content !=
              (message_content_t*) raw_message_t::delimiter_tag &&
              src_->content != (message_content_t*) raw_message_t::vsm_tag) {

            //  One reference is added to shared messages. Non-shared messages
            //  are turned into shared messages and reference count is set to 2.
            if (src_->shared)
               src_->content->refcount.add (1);
            else {
               src_->shared = true;
               src_->content->refcount.set (2);
            }
        }

        *dest_ = *src_;
    }

    //  Returns pointer to the message body.
    inline void *raw_message_data (raw_message_t *msg_)
    {
        if (msg_->content == (message_content_t*) raw_message_t::vsm_tag)
            return msg_->vsm_data;
        if (msg_->content == (message_content_t*) raw_message_t::delimiter_tag)
            return NULL;
        return msg_->content->data;
    }

    //  Returns message size.
    inline size_t raw_message_size (raw_message_t *msg_)
    {
        if (msg_->content == (message_content_t*) raw_message_t::vsm_tag)
            return msg_->vsm_size;
        if (msg_->content == (message_content_t*) raw_message_t::delimiter_tag)
            return 0;
        return msg_->content->size;
    }

}

#endif
