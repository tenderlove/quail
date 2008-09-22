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

#include <arpa/inet.h>

#include "locator.hpp"
#include "bp_listener.hpp"
#include "bp_engine.hpp"
#include "config.hpp"

zmq::locator_t::locator_t (const char *host_)
{
    if (host_) {
        char buff [32];
        snprintf (buff, 32, "%d", (int) default_locator_port);
        global_locator = new tcp_socket_t (host_, NULL, buff);
    }
    else
        global_locator = NULL;

    int rc = pthread_mutex_init (&sync, NULL);
    errno_assert (rc == 0);
}

zmq::locator_t::~locator_t ()
{
    int rc = pthread_mutex_destroy (&sync);
    errno_assert (rc == 0);

    if (global_locator)
        delete global_locator;
}

void zmq::locator_t::create_exchange (const char *exchange_,
    i_context *context_, i_engine *engine_, scope_t scope_,
    const char *interface_, poll_thread_t *listener_thread_,
    int handler_thread_count_, poll_thread_t **handler_threads_)
{
    assert (strlen (exchange_) < 256);

    //  Enter critical section
    int rc = pthread_mutex_lock (&sync);
    errno_assert (rc == 0);

    //  Add the exchange to the list of known exchanges
    exchange_info_t info = {context_, engine_};
    exchanges.insert (exchanges_t::value_type (exchange_, info));

    //  Add exchange to the global locator
    if (scope_ == scope_global) {

         assert (global_locator);
         assert (strlen (interface_) < 256);

         //  Create a listener for the exchange
         bp_listener_t::create (listener_thread_, interface_,
            handler_thread_count_, handler_threads_,
            false, context_, engine_, exchange_);

         //  Send to 'add exchange' command
         unsigned char cmd = create_exchange_id;
         global_locator->blocking_write (&cmd, 1);
         unsigned char size = strlen (exchange_);
         global_locator->blocking_write (&size, 1);
         global_locator->blocking_write (exchange_, size);
         size = strlen (interface_);
         global_locator->blocking_write (&size, 1);
         global_locator->blocking_write (interface_, size);

         //  Read the response
         global_locator->blocking_read (&cmd, 1);
         assert (cmd == create_exchange_ok_id);
    }

    //  Leave critical section
    rc = pthread_mutex_unlock (&sync);
    errno_assert (rc == 0);
}

bool zmq::locator_t::get_exchange (const char *exchange_, i_context **context_,
    i_engine **engine_, poll_thread_t *thread_, const char *local_object_)
{
    //  Enter critical section
    int rc = pthread_mutex_lock (&sync);
    errno_assert (rc == 0);

    //  Find the exchange
    exchanges_t::iterator it = exchanges.find (exchange_);

    //  If the exchange is unknown, find it using global locator
    if (it == exchanges.end ()) {

         //  If we are running without global locator, fail
         if (!global_locator) {

             //  Leave critical section
             rc = pthread_mutex_unlock (&sync);
             errno_assert (rc == 0);

             return false;
         }

         //  Send to 'get exchange' command
         unsigned char cmd = get_exchange_id;
         global_locator->blocking_write (&cmd, 1);
         unsigned char size = strlen (exchange_);
         global_locator->blocking_write (&size, 1);
         global_locator->blocking_write (exchange_, size);

         //  Read the response
         global_locator->blocking_read (&cmd, 1);
         if (cmd == fail_id) {

             //  Leave critical section
             rc = pthread_mutex_unlock (&sync);
             errno_assert (rc == 0);

             return false;
         }

         assert (cmd == get_exchange_ok_id);
         global_locator->blocking_read (&size, 1);
         char interface [256];
         global_locator->blocking_read (interface, size);
         interface [size] = 0;

         //  Create the proxy engine for the exchange
         bp_engine_t *engine = bp_engine_t::create (thread_,
             interface, bp_out_batch_size, bp_in_batch_size,
             local_object_);

         //  Write it into exchange repository
         exchange_info_t info = {thread_, engine};
         it = exchanges.insert (
             exchanges_t::value_type (exchange_, info)).first;
    }

    *context_ = it->second.context;
    *engine_ = it->second.engine;

    //  Leave critical section
    rc = pthread_mutex_unlock (&sync);
    errno_assert (rc == 0);

    return true;
}

void zmq::locator_t::create_queue (const char *queue_, i_context *context_,
    i_engine *engine_, scope_t scope_, const char *interface_,
    poll_thread_t *listener_thread_, int handler_thread_count_,
    poll_thread_t **handler_threads_)
{
    assert (strlen (queue_) < 256);

    //  Enter critical section
    int rc = pthread_mutex_lock (&sync);
    errno_assert (rc == 0);

    queue_info_t info = {context_, engine_};
    queues.insert (queues_t::value_type (queue_, info));

    //  Add queue to the global locator
    if (scope_ == scope_global) {

         assert (global_locator);
         assert (strlen (interface_) < 256);

         //  Create a listener for the exchange
         bp_listener_t::create (listener_thread_, interface_,
            handler_thread_count_, handler_threads_,
            true, context_, engine_, queue_);

         //  Send to 'add queue' command
         unsigned char cmd = create_queue_id;
         global_locator->blocking_write (&cmd, 1);
         unsigned char size = strlen (queue_);
         global_locator->blocking_write (&size, 1);
         global_locator->blocking_write (queue_, size);
         size = strlen (interface_);
         global_locator->blocking_write (&size, 1);
         global_locator->blocking_write (interface_, size);

         //  Read the response
         global_locator->blocking_read (&cmd, 1);
         assert (cmd == create_queue_ok_id);
    }

    //  Leave critical section
    rc = pthread_mutex_unlock (&sync);
    errno_assert (rc == 0);
}

bool zmq::locator_t::get_queue (const char *queue_, i_context **context_,
    i_engine **engine_, poll_thread_t *thread_, const char *local_object_)
{
    //  Enter critical section
    int rc = pthread_mutex_lock (&sync);
    errno_assert (rc == 0);

    queues_t::iterator it = queues.find (queue_);
 
    //  If the exchange is unknown, find it using global locator
    if (it == queues.end ()) {

         //  If we are running without global locator, fail
         if (!global_locator) {

             //  Leave critical section
             rc = pthread_mutex_unlock (&sync);
             errno_assert (rc == 0);

             return false;
         }

         //  Send to 'get queue' command
         unsigned char cmd = get_queue_id;
         global_locator->blocking_write (&cmd, 1);
         unsigned char size = strlen (queue_);
         global_locator->blocking_write (&size, 1);
         global_locator->blocking_write (queue_, size);

         //  Read the response
         global_locator->blocking_read (&cmd, 1);
         if (cmd == fail_id) {

             //  Leave critical section
             rc = pthread_mutex_unlock (&sync);
             errno_assert (rc == 0);

             return false;
         }

         assert (cmd == get_queue_ok_id);
         global_locator->blocking_read (&size, 1);
         char interface [256];
         global_locator->blocking_read (interface, size);
         interface [size] = 0;

         //  Create the proxy engine for the exchange
         bp_engine_t *engine = bp_engine_t::create (thread_,
             interface, bp_out_batch_size, bp_in_batch_size,
             local_object_);

         //  Write it into queue repository
         queue_info_t info = {thread_, engine};
         it = queues.insert (
             queues_t::value_type (queue_, info)).first;
    }

    *context_ = it->second.context;
    *engine_ = it->second.engine;

    //  Leave critical section
    rc = pthread_mutex_unlock (&sync);
    errno_assert (rc == 0);

    return true;
}
