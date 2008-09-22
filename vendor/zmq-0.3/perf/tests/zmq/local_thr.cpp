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
#include "../scenarios/thr.hpp"
#include "../../helpers/functions.hpp"

using namespace std;

int main (int argc, char *argv [])
{
    if (argc != 6) {
        cerr << "Usage: local_thr <hostname> <interface> <message size> "
            "<message count> <number of threads>" << endl;
        return 1;
    }

    //  Parse & print command line arguments.
    const char *host = argv [1];
    const char *interface = argv [2];
    size_t msg_size = atoi (argv [3]);
    int msg_count = atoi (argv [4]);
    int thread_count = atoi (argv [5]);

    cout << "threads: " << thread_count << endl;
    cout << "message size: " << msg_size << " [B]" << endl;
    cout << "message count: " << msg_count << endl;

    //  Create *transports array.
    perf::i_transport **transports = new perf::i_transport* [thread_count];

    //  Create as many transports as threads, each worker thread uses its own
    //  names for queues and exchanges (Q0 and E0, Q1 and E1 ...) Each exchange
    //  or queue is assigned its own port number starting at 5556.
    for (int thread_nbr = 0; thread_nbr < thread_count; thread_nbr++) {

        //  Create queue name Q0, Q1, ...
        string queue_name ("Q");
        queue_name += perf::to_string (thread_nbr);

        //  Create exchange name E0, E1, ...
        string exchange_name ("E");
        exchange_name += perf::to_string (thread_nbr);

        //  Add port number to the interface to get full exchange interface.
        string exchange_interface (interface);
        exchange_interface += ":";
        exchange_interface += perf::to_string (5556 + thread_nbr * 2);

        //  Add port number to the interface to get full queue interface.
        string queue_interface (interface);
        queue_interface += ":";
        queue_interface += perf::to_string (5557 + thread_nbr * 2);

        //  Create zmq transport with bind = false. It means that global queue
        //  QX and global exchange EX will be created without any bindings.
        transports [thread_nbr] = new perf::zmq_t (host, false,
            exchange_name.c_str (), queue_name.c_str (),
            exchange_interface.c_str (), queue_interface.c_str ());
    }

    //  Do the job, for more detailed info refer to ../scenarios/thr.hpp.
    perf::local_thr (transports, msg_size, msg_count, thread_count);
    
    //  Cleanup.
    for (int thread_nbr = 0; thread_nbr < thread_count; thread_nbr++)
        delete transports [thread_nbr];
    
    delete [] transports;

    return 0;
}
