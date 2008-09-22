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

#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "../../transports/zmq.hpp"
#include "../scenarios/thr_lat.hpp"

using namespace std;

int main (int argc, char *argv [])
{

    if (argc != 5) { 
        cerr << "Usage: remote_thr_lat <hostname> <message size> "
            "<message count> <msgs per sec>" << endl; 
        return 1;
    }

    //  Parse & print command line arguments.
    const char *host = argv [1];
    size_t msg_size = atoi (argv [2]);
    int msg_count = atoi (argv [3]);
    int msgs_per_sec = atoi (argv [4]);

    cout << "message size: " << msg_size << " [B]" << endl;
    cout << "message count: " << msg_count << endl;
    cout << "messages per second: " << msgs_per_sec << " [msg/s]" << endl;

    perf::zmq_t transport  (host, true, "E0", "Q0", NULL, NULL);

    perf::remote_thr_lat (&transport, msg_size, msg_count, msgs_per_sec);
    
    return 0;
}

