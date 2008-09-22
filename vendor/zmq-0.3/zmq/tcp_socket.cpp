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

#include "tcp_socket.hpp"

#include <assert.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#include "err.hpp"

zmq::tcp_socket_t::tcp_socket_t (const char *host_,
    const char *default_address_, const char *default_port_)
{
    //  Convert the hostname into sockaddr_in structure.
    sockaddr_in ip_address;
    resolve_ip_address (&ip_address, host_, default_address_, default_port_);

    //  Create the socket
    s = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    errno_assert (s != -1);

    //  Connect to the remote peer
    int rc = connect (s, (sockaddr *)&ip_address, sizeof (ip_address));
    errno_assert (rc != -1);

    //  Disable Nagle's algorithm
    int flag = 1;
    rc = setsockopt (s, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof (int));
    errno_assert (rc == 0);
}

zmq::tcp_socket_t::tcp_socket_t (tcp_listener_t &listener)
{
    //  Accept the socket
    s = listener.accept ();
    errno_assert (s != -1);

    //  Disable Nagle's algorithm
    int flag = 1;
    int rc = setsockopt (s, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof (int));
    errno_assert (rc == 0);
}

zmq::tcp_socket_t::~tcp_socket_t ()
{
    int rc = close (s);
    errno_assert (rc == 0);
}

size_t zmq::tcp_socket_t::write (const void *data, size_t size)
{
    ssize_t nbytes = send (s, data, size, MSG_DONTWAIT);

    //  Signalise peer failure
    if (nbytes == -1 && errno == ECONNRESET)
        return 0;

    errno_assert (nbytes != -1);
    return (size_t) nbytes;
}

size_t zmq::tcp_socket_t::read (void *data, size_t size)
{
    ssize_t nbytes = recv (s, data, size, MSG_DONTWAIT);
    errno_assert (nbytes != -1);
    return (size_t) nbytes;
}

void zmq::tcp_socket_t::blocking_write (const void *data, size_t size)
{
    ssize_t nbytes = send (s, data, size, 0);
    errno_assert (nbytes != -1);
    assert (((size_t) nbytes) == size);
}

void zmq::tcp_socket_t::blocking_read (void *data, size_t size)
{
    ssize_t nbytes = recv (s, data, size, MSG_WAITALL);
    errno_assert (nbytes != -1);
    assert (((size_t) nbytes) == size);
}

