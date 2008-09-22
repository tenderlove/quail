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
    Message flow diagram - fan in scenario

          'local'                 'remote 1'               'remote 2'
         subscriber               publisher                 publisher
      (started first)          (started second)         (started third)
             |
             |  sync message (size 1B) |
             |<------------------------|                        |       
             |                         | sync message (size 1B  |
             |<-------------------------------------------------|
             |                         |                        |
             |  sync message (size 1B) |                        |
             |------------------------>| sync message (size 1B) |
             |------------------------------------------------->|
             |                         |                        | 
             |  messages (size,count)  |                        |
             |<========================|                        |
             |                         |  messages (size,count) |                      
             |<=================================================|
             |                         |                        |
     resuls gathering                  |                        |
       computations                    |                        |
             |                         |                        | 
             |  sync message (size 1B) |                        |
             |------------------------>| sync message (size 1B) |
             |------------------------------------------------->|
             |                         |                        |
             v                         v                        v

*/

#ifndef __FI_HPP_INCLUDED__
#define __FI_HPP_INCLUDED__

#include <assert.h>
#include <iostream>
#include <fstream>
#include "../../transports/i_transport.hpp"
#include "../../helpers/time.hpp"

namespace perf
{
    //  Subscriber function.
    void local_fi (i_transport *transport_, size_t msg_size_, 
          int msg_count_, int pubs_count_)
    {

        //  Wait for sync messages from publishers.
        for (int pubs_nbr = 0; pubs_nbr < pubs_count_; pubs_nbr++) {
            size_t size = transport_->receive ();
            assert (size == 1);
        }

        //  Send sync message to publishers (they can start to send messages).
        transport_->send (1);
       
        time_instant_t start_time = 0;

        //  Receive messages from all publishers.
        for (int msg_nbr = 0; msg_nbr < pubs_count_ * msg_count_; 
            msg_nbr++) {
            
            size_t size = transport_->receive ();

            //  Capture timestamp of the first arrived message.
            if (msg_nbr == 0)
                start_time = now ();
            
            //  Check incomming message size.
            assert (size == msg_size_);
        }

        //  Capture test ent timestamp. 
        time_instant_t stop_time = now (); 
 
        //  Test time in [ms] with [ms] resolution, do not use for math!!!
        uint64_t test_time = (uint64_t) (stop_time - start_time) /
            (uint64_t) 1000000;

        //  Throughput [msgs/s].
        uint64_t msg_thput = ((uint64_t) 1000000000 *
            (uint64_t) msg_count_ * (uint64_t) pubs_count_) /
            (uint64_t) (stop_time - start_time);
          
        //  Throughput [Mb/s].
        uint64_t tcp_thput = (msg_thput * msg_size_ * 8) /
            (uint64_t) 1000000;
                
        std::cout << std::noshowpoint << "Your average throughput is " 
            << msg_thput << " [msg/s]\n";
        std::cout << std::noshowpoint << "Your average throughput is " 
            << tcp_thput << " [Mb/s]\n\n";
 
        //  Save the results into the tests.dat file.
        std::ofstream outf ("tests.dat", std::ios::out | std::ios::app);
        assert (outf.is_open ());
        
        //  Output file format, separate line for each run is appended 
        //  to the tests.dat file.
        //
        //  Publishers count, message count (per publisher), msg size [B], 
        //  test time [ms], throughput [msg/s],throughput [Mb/s].
        //
        outf << pubs_count_ << "," << msg_count_ * pubs_count_ << "," 
            << msg_size_ << "," << test_time << "," << msg_thput << "," 
            << tcp_thput << std::endl;
        
        outf.close ();

        //  Send sysnc message to publishers.
        transport_->send (1);
    }

    //  Publisher function.
    void remote_fi (i_transport *transport_, size_t msg_size_, 
          int roundtrip_count_)
    {
        //  Send sync message to subscriber.
        transport_->send (1); 

        //  Wait for sync message.
        size_t size = transport_->receive ();
        assert (size == 1);

        //  Send a bunch of messages.
        for (int msg_nbr = 0; msg_nbr < roundtrip_count_; msg_nbr++) {
            transport_->send (msg_size_);
        }

        //  Wait for sync message.
        size = transport_->receive ();
        assert (size == 1);
    }
}
#endif
