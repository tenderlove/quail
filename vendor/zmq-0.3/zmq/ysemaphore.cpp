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

#include "ysemaphore.hpp"

#if (defined ZMQ_HAVE_LINUX || defined ZMQ_HAVE_OSX)

void zmq::ysemaphore_t::signal (int signal_)
{
    assert (signal_ == 0);
    int rc = pthread_mutex_unlock (&mutex);
    errno_assert (rc == 0);
}

#else

void zmq::ysemaphore_t::signal (int signal_)
{
    assert (signal_ == 0);
    int rc = sem_post (&sem);
    errno_assert (rc != -1);
}

#endif

