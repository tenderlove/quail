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

#include <algorithm>
#include <assert.h>

#include "dispatcher.hpp"
#include "err.hpp"


zmq::dispatcher_t::dispatcher_t (int thread_count_) :
    thread_count (thread_count_ + 1),
    signalers (thread_count, (i_signaler*) NULL),
    used (thread_count, false)
{
    //  Alocate NxN matrix of dispatching pipes.
    pipes = new command_pipe_t [thread_count * thread_count];
    assert (pipes);

    //  Mark admin thread ID as used.
    used [admin_thread_id] = true;

    //  Initialise the mutex.
    int rc = pthread_mutex_init (&mutex, NULL);
    errno_assert (rc == 0);
}

zmq::dispatcher_t::~dispatcher_t ()
{
    //  Uninitialise the mutex.
    int rc = pthread_mutex_destroy (&mutex);
    errno_assert (rc == 0);

    //  Deallocate the pipe matrix.
    delete [] pipes;
}

int zmq::dispatcher_t::allocate_thread_id (i_signaler *signaler_)
{
    //  Lock the mutex.
    int rc = pthread_mutex_lock (&mutex);
    errno_assert (rc == 0);

    //  Find the first free thread ID.
    std::vector <bool>::iterator it = std::find (used.begin (),
        used.end (), false);

    //  No more thread IDs are available!
    assert (it != used.end ());

    //  Mark the thread ID as used.
    *it = true;
    int thread_id = it - used.begin ();

    //  Unlock the mutex.
    rc = pthread_mutex_unlock (&mutex);
    errno_assert (rc == 0);

    //  Set the signaler.
    signalers [thread_id] = signaler_;

    return thread_id;
}

void zmq::dispatcher_t::deallocate_thread_id (int thread_id_)
{
    //  Lock the mutex.
    int rc = pthread_mutex_lock (&mutex);
    errno_assert (rc == 0);

    //  Free the specified thread ID.
    assert (used [thread_id_] == true);
    used [thread_id_] = false;

    //  Unregister signaler.
    signalers [thread_id_] = NULL;

    //  Unlock the mutex.
    rc = pthread_mutex_unlock (&mutex);
    errno_assert (rc == 0);
}

int zmq::dispatcher_t::get_thread_id ()
{
    return admin_thread_id;
}

void zmq::dispatcher_t::send_command (i_context *destination_,
    const command_t &command_)
{
    //  Lock the mutex.
    int rc = pthread_mutex_lock (&mutex);
    errno_assert (rc == 0);

    //  Pass the command to the other thread via a pipe.
    write (admin_thread_id, destination_->get_thread_id (), command_);

    //  Unlock the mutex.
    rc = pthread_mutex_unlock (&mutex);
    errno_assert (rc == 0);
}

