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

#ifndef __ZMQ_I_LOCATOR_HPP_INCLUDED__
#define __ZMQ_I_LOCATOR_HPP_INCLUDED__

#include "i_context.hpp"
#include "i_engine.hpp"
#include "scope.hpp"
#include "poll_thread.hpp"

namespace zmq
{

    struct i_locator
    {
        //  The destructor shouldn't be virtual, however, not defining it as
        //  such results in compiler warnings with some compilers.
        virtual ~i_locator () {};

        //  Creates an exchange.
        virtual void create_exchange (const char *exchange_,
            i_context *context_, i_engine *engine_, scope_t scope_,
            const char *interface_,
            poll_thread_t *listener_thread_, int handler_thread_count_,
            poll_thread_t **handler_threads_) = 0;

        //  Gets the engine that handles specified exchange.
        //  Returns false if the exchange is unknown.
        virtual bool get_exchange (const char *exchange_,
            i_context **context_, i_engine **engine_,
            class poll_thread_t *thread_, const char *local_object_) = 0;

        //  Creates a queue.
        virtual void create_queue (const char *exchange_,
            i_context *context_, i_engine *engine_, scope_t scope_,
            const char *interface_,
            poll_thread_t *listener_thread_, int handler_thread_count_,
            poll_thread_t **handler_threads_) = 0;

        //  Gets the engine that handles specified queue.
        //  Returns false if the queue is unknown.
        virtual bool get_queue (const char *exchange_,
            i_context **context_, i_engine **engine_,
            class poll_thread_t *thread_, const char *local_object_) = 0;
    };

}

#endif

