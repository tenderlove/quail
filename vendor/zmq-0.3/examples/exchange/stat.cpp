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

#include <stdio.h>
#include <queue>

#include "../../zmq/dispatcher.hpp"
#include "../../zmq/locator.hpp"
#include "../../zmq/api_thread.hpp"
#include "../../zmq/poll_thread.hpp"
#include "../../zmq/message.hpp"

#include "messages.hpp"
using namespace exchange;

//  Class to handle incoming statistics
class handler_t
{
public:

    inline handler_t ()
    {
    }

    inline void order (order_id_t order_id_, order_type_t type_,
        price_t price_, volume_t volume_)
    {
        assert (false);
    }

    inline void order_confirmation (order_id_t order_id_)
    {
        assert (false);
    }

    inline void trade (order_id_t order_id_, price_t price_, volume_t volume_)
    {
        assert (false);
    }

    inline void quote (price_t bid_, price_t ask_)
    {
        assert (false);
    }

    inline void throughput (uint8_t meter_id_, uint64_t throughput_)
    {
        printf ("%1d:%08lu\n", (int) meter_id_, (unsigned long) throughput_);
        fflush (stdout);
    }

    inline void timestamp (uint8_t meter_id_, uint64_t correlation_id_,
        uint64_t timestamp_)
    {
        if (meter_id_ == 5) {
            if (!confirmation_timestamps.empty ()) {
                printf ("l:%08lu\n",
                    (unsigned long) (confirmation_timestamps.front () -
                    timestamp_));
                fflush (stdout);
                confirmation_timestamps.pop ();
            }
            else
                order_timestamps.push (timestamp_);
            return;
        }

        if (meter_id_ == 6) {
            if (!order_timestamps.empty ()) {
                printf ("l:%08lu\n",
                    ((unsigned long) (timestamp_ - order_timestamps.front ())));
                fflush (stdout);
                order_timestamps.pop ();
            }
            else
                confirmation_timestamps.push (timestamp_);
            return;
        }
    }

private:

    std::queue <uint64_t> order_timestamps;
    std::queue <uint64_t> confirmation_timestamps;
};

int main (int argc, char *argv [])
{
    if (argc != 3) {
        printf ("Usage: stat <hostname> <interface>\n");
        return 1;
    }

    //  Initialise 0MQ infrastructure
    zmq::dispatcher_t dispatcher (2);
    zmq::locator_t locator (argv [1]);
    zmq::api_thread_t *api = zmq::api_thread_t::create (&dispatcher, &locator);
    zmq::poll_thread_t *pt = zmq::poll_thread_t::create (&dispatcher);

    //  Initialise the wiring
    api->create_queue ("SQ", zmq::scope_global, argv [2], pt, 1, &pt);

    //  Handler object
    handler_t handler;

    //  Message dispatch loop
    while (true) {
        zmq::message_t msg;
        api->receive (&msg);
        parse_message (&msg, &handler);
    } 

    return 0;
}
