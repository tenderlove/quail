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
#include <cstdio>
#include <iostream>

#include "../../transports/tcp.hpp"
#include "../scenarios/thr.hpp"

using namespace std;

int main (int argc, char *argv [])
{
    if (argc != 6) { 
        cerr << "Usage: remote_thr <IP address of \'local\'> <\'local\' port> "
            << "<message size> <message count> <number of threads>\n"; 
        return 1;
    }
 
    //  Parse & print arguments.
    const char *peer_ip = argv [1];
    unsigned short peer_port = atoi (argv [2]);

    int thread_count = atoi (argv [5]);
    size_t msg_size = atoi (argv [3]);
    int msg_count = atoi (argv [4]);

    cout << "threads: " << thread_count << endl;
    cout << "message size: " << msg_size << " [B]" << endl;
    cout << "message count: " << msg_count << endl << endl;

    //  Create *transports array.
    perf::i_transport **transports = new perf::i_transport* [thread_count];

    //  Create as many transports as threads, each worker thread uses own
    //  transport listen port increases by 1.
    for (int thread_nbr = 0; thread_nbr < thread_count; thread_nbr++)
    {
        transports [thread_nbr] = new perf::tcp_t (false, peer_ip, 
            peer_port + thread_nbr, false);

        //  Give time to the peer to start to listen.
        sleep (1);
    }

    //  Do the job, for more detailed info refer to ../scenarios/thr.hpp.
    perf::remote_thr (transports, msg_size, msg_count, thread_count);
   
    //  Cleanup.
    for (int thread_nbr = 0; thread_nbr < thread_count; thread_nbr++)
    {
        delete transports [thread_nbr];
    }
    
    delete [] transports;

    return 0;
}
