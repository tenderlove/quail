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

#ifndef __ZMQ_ATOMIC_COUNTER_HPP_INCLUDED__
#define __ZMQ_ATOMIC_COUNTER_HPP_INCLUDED__

#include <pthread.h>

#include "err.hpp"
#include "stdint.hpp"

namespace zmq
{

    //  This class represents an integer that can be incremented/decremented
    //  in atomic fashion.

    class atomic_counter_t
    {
    public:

        typedef uint32_t integer_t;

        inline atomic_counter_t (integer_t value_ = 0) :
            value (value_)
        {
#if (defined (ZMQ_FORCE_MUTEXES) || !defined (__GNUC__) || (!defined (__i386__)\
    && !defined (__x86_64__)))
            int rc = pthread_mutex_init (&mutex, NULL);
            errno_assert (rc == 0);
#endif
        }

        inline ~atomic_counter_t ()
        {
#if (defined (ZMQ_FORCE_MUTEXES) || !defined (__GNUC__) || (!defined (__i386__)\
    && !defined (__x86_64__)))
            int rc = pthread_mutex_destroy (&mutex);
            errno_assert (rc == 0);
#endif
        }

        //  Set counter value (not thread-safe).
        inline void set (integer_t value_)
        {
            value = value_;
        }

        //  Atomic addition. Returns false if counter was zero
        //  before the operation.
        inline bool add (integer_t increment)
        {
#if (!defined (ZMQ_FORCE_MUTEXES) && (defined (__i386__) ||\
    defined (__x86_64__)) && defined (__GNUC__))
            volatile integer_t *val = &value;
            __asm__ volatile ("lock; xaddl %0,%1"
                : "=r" (increment), "=m" (*val)
                : "0" (increment), "m" (*val)
                : "memory", "cc");
            return increment;
#else
            int rc = pthread_mutex_lock (&mutex);
            errno_assert (rc == 0);
            bool result = value;
            value += increment;
            rc = pthread_mutex_unlock (&mutex);
            errno_assert (rc == 0);
            return result;
#endif
        }

        //  Atomic subtraction. Returns false if the counter drops to zero.
        inline bool sub (integer_t decrement)
        {
#if (!defined (ZMQ_FORCE_MUTEXES) && (defined (__i386__) ||\
    defined (__x86_64__)) && defined (__GNUC__))
            integer_t oldval = -decrement;
            volatile integer_t *val = &value;
            __asm__ volatile ("lock; xaddl %0,%1"
                : "=r" (oldval), "=m" (*val)
                : "0" (oldval), "m" (*val)
                : "memory", "cc");
            return oldval != decrement;
#else
            int rc = pthread_mutex_lock (&mutex);
            errno_assert (rc == 0);
            value -= decrement;
            bool result = value;
            rc = pthread_mutex_unlock (&mutex);
            errno_assert (rc == 0);
            return result;
#endif
        }

    protected:

        volatile integer_t value;
#if (defined (ZMQ_FORCE_MUTEXES) || !defined (__GNUC__) ||\
    (!defined (__i386__) && !defined (__x86_64__)))
        pthread_mutex_t mutex;
#endif
    };

}

#endif
