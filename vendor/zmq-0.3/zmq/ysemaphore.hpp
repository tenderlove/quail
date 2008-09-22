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

#ifndef __ZMQ_YSEMAPHORE_HPP_INCLUDED__
#define __ZMQ_YSEMAPHORE_HPP_INCLUDED__

#include <assert.h>
#include <pthread.h>
#if (!defined ZMQ_HAVE_LINUX && !defined ZMQ_HAVE_OSX)
    #include <semaphore.h>
#endif

#include "platform.hpp"
#include "i_signaler.hpp"
#include "err.hpp"

namespace zmq
{

    //  Simple semaphore. Only single thread may be waiting at any given time.
    //  Also, the semaphore may not be posted before the previous post
    //  was matched by corresponding wait and the waiting thread was
    //  released.
    //
    //  ysemaphore is designed for platforms where both mutex locking
    //  and unlocking provides full memory fence.
    //  On the platforms where lock is implemented using one-way acquire-only
    //  memory barrier and/or unlock uses release-only memory barrier,
    //  real semaphore should be used instead (or a lock extended by release
    //  barrier and unlock extended by acquire barrier).
    //
    //  Implementation notes:
    //
    //  - on FreeBSD platform mutex cannot be locked twice from the same thread;
    //    therefore real semaphore should be used
    //  - on OS X unnamed semaphores are not supported, therefore mutex-based
    //    substitute should be used

#if (defined ZMQ_HAVE_LINUX || defined ZMQ_HAVE_OSX)

    class ysemaphore_t : public i_signaler
    { 
    public:

        //  Initialise the semaphore.
        inline ysemaphore_t ()
        {
            int rc = pthread_mutex_init (&mutex, NULL);
	    errno_assert (rc == 0);
            rc = pthread_mutex_lock (&mutex);
	    errno_assert (rc == 0);
        }

        //  Destroy the semaphore.
        inline ~ysemaphore_t ()
        {
            int rc = pthread_mutex_unlock (&mutex);
	    errno_assert (rc == 0);
            rc = pthread_mutex_destroy (&mutex);
	    errno_assert (rc == 0);
        }

        //  Wait for the semaphore.
        inline void wait ()
        {
             int rc = pthread_mutex_lock (&mutex);
             errno_assert (rc == 0);
        }

        //  Post the semaphore.
        void signal (int signal_);

    private:

        //  Simple semaphore is implemented by mutex, as it is more efficient
        //  on Linux platform.
        pthread_mutex_t mutex;
    };

#else

    class ysemaphore_t : public i_signaler
    { 
    public:

        //  Initialise the semaphore.
        inline ysemaphore_t ()
        {
             int rc = sem_init (&sem, 0, 0);
             errno_assert (rc != -1);
        }

        //  Destroy the semaphore.
        inline ~ysemaphore_t ()
        {
             int rc = sem_destroy (&sem);
             errno_assert (rc != -1);
        }

        //  Wait for the semaphore.
        inline void wait ()
        {
             int rc = sem_wait (&sem);
             errno_assert (rc != -1);
        }

        //  Post the semaphore.
        void signal (int signal_);

    private:

        //  Underlying system semaphore object.
        sem_t sem;
    };

#endif

}

#endif
