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

#ifndef __ZMQ_ERR_HPP_INCLUDED__
#define __ZMQ_ERR_HPP_INCLUDED__

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

//  Provides convenient way to check for errno-style errors.
#define errno_assert(x) if (!(x)) {\
    perror (NULL);\
    printf ("%s (%s:%d)\n", #x, __FILE__, __LINE__);\
    abort ();\
}

// Provides convenient way to check for errors from getaddrinfo.
#define gai_assert(x) if (x) {\
    const char *errstr = gai_strerror (x);\
    printf ("%s (%s:%d)\n", errstr, __FILE__, __LINE__);\
    abort ();\
}

#endif
