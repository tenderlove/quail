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

    Message flow diagram for throughput scenario

          'local'                   'remote'
      (started first)          (started second)
             |
             |                        
             |                         |
             |  messages (size,count)  | 
             |<========================|
             |                         |
             | sync message (size 1B)  |
             |------------------------>|
             |                         |
             |                         v
      resuls gathering 
        computations
             |
             v

*/

#ifndef __PERF_THR_HPP_INCLUDED__
#define __PERF_THR_HPP_INCLUDED__

#include <iostream>
#include <fstream>
#include <limits>
#include <pthread.h>

#include "../../transports/i_transport.hpp"
#include "../../helpers/time.hpp"

namespace perf
{
    //  Worker thread arguments structure.
    struct thr_worker_args_t
    {
        //  Transport beeing used by the worker, it has to be created
        //  in advance.
        i_transport *transport;

        //  Size of the message being transported in the test.
        size_t msg_size;

        //  Number of the messages in the test.
        int msg_count;

        //  Timestamps captured by the worker thread at the beggining & end 
        //  of the test.
        time_instant_t start_time;
        time_instant_t stop_time;
    };

    //  'Local' worker thread function.
    void *local_worker_function (void *worker_args_)
    {
        thr_worker_args_t *args = (thr_worker_args_t*)worker_args_;

        //  Receive msg_nbr messages of msg_size.
        for (int msg_nbr = 0; msg_nbr < args->msg_count; msg_nbr++)
        {
            size_t size = args->transport->receive ();

            //  Capture arrival timestamp of the first message (test start).
            if (msg_nbr == 0)
                args->start_time  = now ();
            
            //  Check incomming message size.
            assert (size == args->msg_size);
        }

        //  Capture test stop timestamp.
        args->stop_time = now();

        //  Send sync message to the peer.
        args->transport->send (1);

        return NULL;
    }

    //  'Remote' worker thread function.
    void *remote_worker_function (void *worker_args_)
    {
        thr_worker_args_t *args = (thr_worker_args_t*)worker_args_;

        //  Send msg_nbr messages of msg_size.
        for (int msg_nbr = 0; msg_nbr < args->msg_count; msg_nbr++)
        {
            args->transport->send (args->msg_size);
        }

        //  Wait for sync message.
        size_t size = args->transport->receive ();
        assert (size == 1);

        return NULL;
    }

    //  Function initializes parameter structure for each thread and starts
    //  local_worker_function(s) in separate thread(s).
    void local_thr (i_transport **transports_, size_t msg_size_, 
        int msg_count_, int thread_count_)
    {
        pthread_t *workers = new pthread_t [thread_count_];

        //  Array of thr_worker_args_t structures for worker threads.
        thr_worker_args_t *workers_args = 
            new thr_worker_args_t [thread_count_];

        for (int thread_nbr = 0; thread_nbr < thread_count_; thread_nbr++) {

            //  Fill structure, note that start_time & stop_time is filled 
            //  by worker thread at the begining & end of the test.
            workers_args [thread_nbr].transport = transports_ [thread_nbr];
            workers_args [thread_nbr].msg_size = msg_size_;
            workers_args [thread_nbr].msg_count = msg_count_;
            workers_args [thread_nbr].start_time = 0;
            workers_args [thread_nbr].stop_time = 0;
            
            //  Create worker thread.
            int rc = pthread_create (&workers [thread_nbr], NULL, 
                local_worker_function, (void *)&workers_args [thread_nbr]);
            assert (rc == 0);
        }

        //  Gather results from thr_worker_args_t structures.
        time_instant_t min_start_time  = 
            std::numeric_limits<uint64_t>::max ();
        time_instant_t max_stop_time = 0;

        for (int thread_nbr = 0; thread_nbr < thread_count_; thread_nbr++) {

            //  Wait for worker threads to finish.
            int rc = pthread_join (workers [thread_nbr], NULL);
            assert (rc == 0);

            //  Find max stop & min start time.
            if (workers_args [thread_nbr].start_time < min_start_time)
                min_start_time = workers_args [thread_nbr].start_time;

            if (workers_args [thread_nbr].stop_time > max_stop_time)
                max_stop_time = workers_args [thread_nbr].stop_time;

        }

        delete [] workers_args;
        delete [] workers;

        //  Calculate results.

        //  Test time in [ms] with [ms] resolution, do not use for math!!!
        uint64_t test_time = uint64_t (max_stop_time - min_start_time) /
            (uint64_t) 1000000;

        //  Throughput [msgs/s].
        uint64_t msg_thput = ((uint64_t) 1000000000 *
            (uint64_t) msg_count_ * (uint64_t) thread_count_) /
            (uint64_t) (max_stop_time - min_start_time);

        //  Throughput [Mb/s].
        uint64_t tcp_thput = (msg_thput * msg_size_ * 8) /
            (uint64_t) 1000000;
                
        std::cout << "Your average throughput is " << msg_thput 
            << " [msg/s]" << std::endl;
        std::cout << "Your average throughput is " << tcp_thput 
            << " [Mb/s]" << std::endl << std::endl;
 
        //  Save the results into tests.dat file.
        std::ofstream outf ("tests.dat", std::ios::out | std::ios::app);
        assert (outf.is_open ());
        
        //  Output file format, separate line for each run is appended 
        //  to the tests.dat file.
        //
        //  thread count, message count, msg size [B], test time [ms],
        //  throughput [msg/s],throughput [Mb/s]
        //
        outf << thread_count_ << "," << msg_count_ << "," << msg_size_ << "," 
            << test_time << "," << msg_thput << "," << tcp_thput << std::endl;
        
        outf.close ();
    }

    //  Function initializes parameter structure for each thread and starts
    //  remote_worker_function(s) in separate thread(s).
    void remote_thr (i_transport **transports_, size_t msg_size_, 
        int msg_count_, int thread_count_)
    {
        pthread_t *workers = new pthread_t [thread_count_];

        //  Array of thr_worker_args_t structures for worker threads.
        thr_worker_args_t *workers_args = 
            new thr_worker_args_t [thread_count_];


        for (int thread_nbr = 0; thread_nbr < thread_count_; thread_nbr++) {
            //  Fill structures.
            workers_args [thread_nbr].transport = transports_ [thread_nbr];
            workers_args [thread_nbr].msg_size = msg_size_;
            workers_args [thread_nbr].msg_count = msg_count_;
            workers_args [thread_nbr].start_time = 0;
            workers_args [thread_nbr].stop_time = 0;
         
            // Create worker thread.
            int rc = pthread_create (&workers [thread_nbr], NULL, 
                remote_worker_function, (void *)&workers_args [thread_nbr]);
            assert (rc == 0);
        }

        //  Wait for worker threads to finish.
        for (int thread_nbr = 0; thread_nbr < thread_count_; thread_nbr++) {
            int rc = pthread_join (workers [thread_nbr], NULL);
            assert (rc == 0);
        }
    }
}
#endif
