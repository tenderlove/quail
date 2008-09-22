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

/*
    Message flow diagram - fan out scenario

          'local'                  'remote 1'             'remote 2'
         publisher                 subscriber             subscriber
      (started first)          (started second)         (started third)
             |
             |  sync message (size 1B) |
             |<------------------------|                        |       
             |                         | sync message (size 1B  |
             |<-------------------------------------------------|
             |                         |                        |
             |  messages (size,count)  |                        |
             |========================>| messages (size,count)  |
             |=================================================>|
             |                         |                        |
             |                  resuls gathering        resuls gathering 
             |                    computations            computations
             |                         |                        | 
             |  sync message (size 1B) |                        |
             |<------------------------|                        |       
             |                         | sync message (size 1B) |
             |<-------------------------------------------------|
             |                         |                        |
             |  sync message (size 1B) |                        |
             |------------------------>| sync message (size 1B) |
             |------------------------------------------------->|
             |                         |                        |
             v                         v                        v

*/

#ifndef __FO_HPP_INCLUDED__
#define __FO_HPP_INCLUDED__

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <assert.h>

#include "../../transports/i_transport.hpp"
#include "../../helpers/time.hpp"

namespace perf
{
    //  Publisher function.
    void local_fo (i_transport *transport_, size_t msg_size_, 
        int msg_count_, int subs_count)
    {

        //  Receive sync message from each subscriber (they are ready now).
        for (int subs_nbr = 0; subs_nbr < subs_count; subs_nbr++) {
            size_t size = transport_->receive ();
            assert (size == 1);
        }

        //  Send test messages of msg_size_ msg_count_ times.
        for (int msg_nbr = 0; msg_nbr < msg_count_; msg_nbr++) {
            transport_->send (msg_size_);
        }
        
        //  Receive synnc message from each subscriber that they 
        //  received msg_count_ of messages.
        for (int subs_nbr = 0; subs_nbr < subs_count; subs_nbr++) {
            size_t size = transport_->receive ();
            assert (size == 1);
        }
        
        //  Send sync message that subscribers can exit.
        transport_->send (1);
    }

    //  Subscriber function.
    void remote_fo (i_transport *transport_, size_t msg_size_,
        int msg_count_, const char *subs_id_)
    {

        //  Send sync message that we are ready to receive test messages.
        transport_->send (1);

        time_instant_t start_time = 0;

        for (int msg_nbr = 0; msg_nbr < msg_count_; msg_nbr++) {

            size_t size = transport_->receive ();
            //  Capture test start timestamp.
            if (msg_nbr == 0)
                start_time = now();

            //  Check incomming message size.
            assert (size == msg_size_);
        }

        //  Capture test end timestamp.
        time_instant_t stop_time = now();

        //  Calculate results.

        //  Test time in [ms] with [ms] resolution, do not use for math!!!
        uint64_t test_time = (uint64_t) (stop_time - start_time) /
            (uint64_t) 1000000;

        //   Throughput [msgs/s].
        uint64_t msg_thput = ((uint64_t) 1000000000 *
            (uint64_t) msg_count_) /
            (uint64_t)(stop_time - start_time);
        
        //  Throughput [Mb/s].
        uint64_t tcp_thput = (msg_thput * msg_size_ * 8) /
            (uint64_t) 1000000;
 
        //  Save the results into tests.dat file.     
        std::cout << subs_id_ << ": Your average throughput is " 
            << msg_thput << " [msg/s]\n";
        std::cout << subs_id_ << "Your average throughput is " 
            << tcp_thput << " [Mb/s]\n\n";
           
        //  Save the results into ${subs_id}_tests.dat file.
        std::string _filename (subs_id_);
        _filename += "_tests.dat";

        std::ofstream outf (_filename.c_str (), std::ios::out | std::ios::app);
        assert (outf.is_open ());
        
        //  Output file format, separate line for each run is appended 
        //  to the ${subs_id}_tests.dat file
        //
        //  1, message count, msg size [B], test time [ms],
        //  throughput [msg/s],throughput [Mb/s]
        //
        outf << "1," << msg_count_ << "," << msg_size_ << "," << test_time 
            << "," << msg_thput << "," << tcp_thput << std::endl;
        
        outf.close ();

        //  Send sync message.
        transport_->send (1);    

        //  All subscribers finished, now we can shutdown.
        size_t size = transport_->receive ();
        assert (size == 1);
    }
}
#endif
