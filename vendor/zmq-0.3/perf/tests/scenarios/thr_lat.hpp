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
    Message flow diagram for throuthput - latancy scenario

          'local'                   'remote'
      (started first)          (started second)
             |
             |                         | 
             |                         |
             |  messages (size,count)  | 
             |<========================|
         compute, save               save
          results                   results
             |                         |
             | sync message (size 1B)  |
             |------------------------>|
             |                         |
             | sync message (size 1B)  |
             |<------------------------|
             |                         |
             |                         v
             v
*/

#ifndef __PERF_THR_LAT_HPP_INCLUDED__
#define __PERF_THR_LAT_HPP_INCLUDED__

#include <assert.h>
#include <fstream>

#include "../../transports/i_transport.hpp"
#include "../../helpers/ticker.hpp"

namespace perf
{

    void local_thr_lat (i_transport *transport_, size_t msg_size_, 
        int msg_count_)
    {
       
        //  Allocate array for timestamps captured after receive message.
        //  Note that it is neessary to keep time between peers in sync with
        //  accuracy in orders of us. Precision Time protocol (PTP) as defined
        //  by the IEEE 1588 can be used. 
        perf::time_instant_t *stop_times =
            new perf::time_instant_t [msg_count_];

        for (int msg_nbr = 0; msg_nbr < msg_count_; msg_nbr++) {

            //  Receive test message from peer.
            size_t size = transport_->receive ();

            //  Capture timestamp.
            stop_times [msg_nbr] = perf::now ();

            //  Check incomming message size.
            assert (size == msg_size_);
        }

        //  Write stop_times into the stop_times.dat file.
        std::ofstream outf ("stop_times.dat", std::ios::out | std::ios::app);
        assert (outf.is_open ());

        for (int msg_nbr = 0; msg_nbr < msg_count_; msg_nbr++)
        {
            outf << stop_times [msg_nbr] << std::endl;
        }

        outf.close ();

        //  Calculate incomming throughput [msg/s].
        uint64_t msg_thput = ((uint64_t) 1000000000 *
            (uint64_t) msg_count_)/
            (uint64_t) (stop_times [msg_count_ - 1] - stop_times [0]);
           
        //  Calculate throughput [Mb/s].
        uint64_t tcp_thput = (msg_thput * msg_size_ * 8) /
            (uint64_t) 1000000;
                
        std::cout << "Your average throughput (incoming) is " 
            << msg_thput << " [msg/s]\n";
        std::cout << "Your average throughput (incoming) is " 
            << tcp_thput << " [Mb/s]\n\n";

        //  Send sync message.
        transport_->send (1);

        //  Wait for peer to write start_times.
        size_t size = transport_->receive ();
        assert (size == 1);

        //  Cleanup.
        delete [] stop_times;
    }

    void remote_thr_lat (i_transport *transport_, size_t msg_size_, 
        int msg_count_, int msgs_per_second_)
    {
        //  Initialize ticker with msgs_per_second ticks frequency.
        ticker_t ticker (msgs_per_second_); 

        //  Allocate array for timestamps captured before sending message.
        //  Note that it is neessary to keep time between peers in sync with
        //  accuracy in orders of us. Precision Time protocol (PTP) as defined
        //  by the IEEE 1588 can be used. 
        perf::time_instant_t *start_times = new perf::time_instant_t [msg_count_];

        for (int msg_nbr = 0; msg_nbr < msg_count_; msg_nbr++)
        { 
            //  Wait for specific ammount of time to 
            //  achieve msgs_per_second_ freq.
            ticker.wait_for_tick ();

            //  Capture timestamp.
            start_times [msg_nbr] = perf::now ();

            //  Sent test message to the peer.
            transport_->send (msg_size_);
        }

        //  Write start_times into the start_times.dat file.
        std::ofstream outf ("start_times.dat", std::ios::out | std::ios::app);
        assert (outf.is_open ());

        for (int msg_nbr = 0; msg_nbr < msg_count_; msg_nbr++)
        {
            outf << start_times [msg_nbr] << std::endl;
        }

        outf.close ();

        //  Wait for sync message from peer.
        size_t size = transport_->receive ();
        assert (size == 1);

        //  Send sync message.
        transport_->send (1);

        delete [] start_times;
    }
}
#endif
