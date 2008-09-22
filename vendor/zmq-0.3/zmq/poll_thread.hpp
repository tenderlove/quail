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

#ifndef __ZMQ_POLL_THREAD_HPP_INCLUDED__
#define __ZMQ_POLL_THREAD_HPP_INCLUDED__

#include <stddef.h>
#include <assert.h>
#include <pthread.h>
#include <poll.h>

#include "i_context.hpp"
#include "i_pollable.hpp"
#include "dispatcher.hpp"
#include "ysocketpair.hpp"

namespace zmq
{

    //  Poll thread is a I/O thread that waits for events from engines
    //  using POSIX poll function and schedules handling of the signals
    //  by individual engines. Engine compatible with poll thread should
    //  expose i_pollable interface.

    class poll_thread_t : public i_context
    {
    public:

        //  Create a poll thread.
        static poll_thread_t *create (dispatcher_t *dispatcher_);

        //  Destroy the poll thread.
        ~poll_thread_t ();

        //  Registers the engine with the poll thread.
        void register_engine (i_pollable *engine_);

        //  Unregisters the engine from the thread.
        void unregister_engine (i_pollable* engine_);

        //  i_context implementation
        int get_thread_id ();
        void send_command (i_context *destination_, const command_t &command_);

    private:

        poll_thread_t (dispatcher_t *dispatcher_);

        //  Main worker thread routine.
        static void *worker_routine (void *arg_);

        //  Main routine (non-static) - called from worker_routine.
        void loop ();

        //  Processes commands from other threads. Returns false if the thread
        //  should terminate.
        bool process_commands (uint32_t signals_);

        //  Pointer to dispatcher.
        dispatcher_t *dispatcher;

        //  Thread ID allocated for the poll thread by dispatcher.
        int thread_id;

        //  Poll thread gets notifications about incoming commands using
        //  this socketpair.
        ysocketpair_t signaler;

        //  Handle of the physical thread doing the I/O work.
        pthread_t worker;

        //  Pollset to pass to the poll function.
        typedef std::vector <pollfd> pollset_t;
        pollset_t pollset;

        //  List of engines handled by this poll thread.
        typedef std::vector <i_pollable*> engines_t;
        engines_t engines;
    };

}

#endif
