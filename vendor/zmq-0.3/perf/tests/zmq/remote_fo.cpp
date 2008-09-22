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

#include <cstdlib>
#include <iostream>

#include "../../transports/zmq.hpp"
#include "../scenarios/fo.hpp"

using namespace std;

int main (int argc, char *argv [])
{
    if (argc != 5) {
        cerr << "Usage: remote_fo <hostname> <message size> <message count> "
            "<subscriber id>" << endl;
        return 1;
    }

    //  Parse & print command line arguments.
    const char *host = argv [1];
    size_t msg_size = atoi (argv [2]);
    int msg_count = atoi (argv [3]);
    const char *subs_id = argv [4];

    cout << "subscriber ID: " << subs_id << endl;
    cout << "message size: " << msg_size << " [B]" << endl;
    cout << "message count: " << msg_count << endl;

    //  Create zmq transport with bind = true. It means that local 
    //  exchange will be created and binded to the global queue QIN and created 
    //  local queue will be binded to global exchange EOUT. 
    //  Global queue and exchange have to be created before (by the local_fo).
    perf::zmq_t transport (host, true, "EOUT", "QIN", NULL, NULL);

    //  Do the job, for more detailed info refer to ../scenarios/fo.hpp.
    perf::remote_fo (&transport, msg_size, msg_count, subs_id);

    return 0;
}
