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

#include "tcp_listener.hpp"

#include <assert.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <string>

#include "err.hpp"

void zmq::resolve_ip_address (sockaddr_in *address_, const char *host_,
    const char *default_address_, const char* default_port_)
{
    const char *port;
    const char *address;
    std::string host;

    if (!host_) {

        //  If host is not given, use default values.
        assert (default_port_);
        assert (default_address_);
        port = default_port_;
        address = default_address_;
    }
    else {

        //  Delimit the IP address portion and port portion of the host string.
        host = host_;
        std::string::size_type pos = host.find (':');

        //  Determine port to use.
        if (pos == std::string::npos) {
            assert (default_port_);
            port = default_port_;
        }
        else
            port = host.c_str () + pos + 1;

        //  Determine IP address to use.
        if (pos == 0) {
            assert (default_address_);
            address = default_address_;
        }
        else {
            host [pos] = 0;
            address = host.c_str ();
        }
    }

    //  Resolve the host name and service name.
    struct addrinfo req;
    memset (&req, 0, sizeof req);
    struct addrinfo *res;
    req.ai_family = AF_INET;
    int rc = getaddrinfo (address, NULL, &req, &res);
    gai_assert (rc);
    *address_ = *((sockaddr_in *) res->ai_addr);
    address_->sin_port = htons (atoi (port));
    freeaddrinfo (res);
}

zmq::tcp_listener_t::tcp_listener_t (const char *host_,
    const char *default_address_, const char *default_port_)
{
    //  Convert the hostname into sockaddr_in structure.
    sockaddr_in ip_address;
    resolve_ip_address (&ip_address, host_, default_address_, default_port_);

    //  Create a listening socket.
    s = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    errno_assert (s != -1);

    //  Allow reusing of the address.
    int flag = 1;
    int rc = setsockopt (s, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof (int));
    errno_assert (rc == 0);

    //  Bind the socket to the network interface and port.
    rc = bind (s, (struct sockaddr*) &ip_address, 
        sizeof (ip_address));
    errno_assert (rc == 0);
              
    //  Listen for incomming connections.
    rc = listen (s, 1);
    errno_assert (rc == 0);
}

int zmq::tcp_listener_t::accept ()
{
    //  Accept one incoming connection.
    int res = ::accept (s, NULL, NULL);
    errno_assert (res != -1);
    return res;
}
