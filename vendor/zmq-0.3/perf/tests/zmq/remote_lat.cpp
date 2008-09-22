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
#include <iostream>

#include "../../transports/zmq.hpp"
#include "../scenarios/lat.hpp"

using namespace std;

int main (int argc, char *argv [])
{
    if (argc != 4) {
        cerr << "Usage: remote <hostname> <message size> <roundtrip count>"
            << endl;
        return 1;
    }

    //  Parse & print command line arguments.
    const char *host = argv [1];
    size_t msg_size = atoi (argv [2]);
    int roundtrip_count = atoi (argv [3]);

    cout << "message size: " << msg_size << " [B]" << endl;
    cout << "roundtrip count: " << roundtrip_count << endl << endl;

    //  Create zmq transport.
    perf::zmq_t transport (host, true, "EOUT", "QIN", NULL, NULL);

    //  Do the job, for more detailed info refer to ../scenarios/lat.hpp.
    perf::remote_lat (&transport, msg_size, roundtrip_count); 

    return 0;
}
