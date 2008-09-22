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

#include <iostream>
#include <cstdlib>
#include <cstdio>

#include "../../transports/zmq.hpp"
#include "../scenarios/thr_lat.hpp"

using namespace std;

int main (int argc, char *argv [])
{
    if (argc != 6) {
        cerr << "Usage: local_thr_lat <hostname> <exchange interface> "
            "<queue interface> <message size> <message count>" << endl;
        return 1;
    }

    //  Parse & print command line arguments.
    const char *host = argv [1];
    const char *exchange_interface = argv [2];
    const char *queue_interface = argv [3];
    size_t msg_size = atoi (argv [4]);
    int msg_count = atoi (argv [5]);

    cout << "message size: " << msg_size << " [B]" << endl;
    cout << "message count: " << msg_count << endl;

    perf::zmq_t transport (host, false, "E0", "Q0",
        exchange_interface, queue_interface);

    perf::local_thr_lat (&transport, msg_size, msg_count);
    
    return 0;
}
