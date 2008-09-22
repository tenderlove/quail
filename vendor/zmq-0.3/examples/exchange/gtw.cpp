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

#include "../../zmq/dispatcher.hpp"
#include "../../zmq/locator.hpp"
#include "../../zmq/api_thread.hpp"
#include "../../zmq/poll_thread.hpp"
#include "../../zmq/message.hpp"

#include "../../perf/helpers/ticker.hpp"
using namespace perf;

#include "messages.hpp"
#include "frequency_meter.hpp"
using namespace exchange;

//  Sender half of the gateway application
//  i.e. Automated order feeder sending orders to the matching engine

class sender_t
{
public:

    inline sender_t (zmq::dispatcher_t *dispatcher_, zmq::locator_t *locator_) :
        meter (500000, 1)
    {
        //  Initialise 0MQ infrastructure
        api = zmq::api_thread_t::create (dispatcher_, locator_);
        pt = zmq::poll_thread_t::create (dispatcher_);

        //  Initialise the wiring
        oe_id = api->create_exchange ("OE");
        api->bind ("OE", "OQ", pt, pt);
        se_id = api->create_exchange ("SE");
        api->bind ("SE", "SQ", pt, pt);
    }

    void run (uint64_t frequency_)
    {
        //  Initialise the ticker
        ticker_t ticker (frequency_);

        order_id_t order_id = 0;
        while (true) {

            //  Delay a bit to get stream of orders with stable throughput
            ticker.wait_for_tick ();

            //  Create random order
            order_id ++;
            order_type_t type = (random () % 2) ? ask : bid;
            price_t price = random () % 100 + 450;
            volume_t volume = random () % 100 + 1;

            //  Send a timestamp to the stat component
            if (order_id % 500000 == 0) {
                zmq::message_t msg;
                make_timestamp (5, order_id, perf::now () / 1000, &msg);
                api->send (se_id, msg);
            }

            //  Send the order to the matching engine
            zmq::message_t msg;
            make_order (order_id, type, price, volume, &msg);
            api->send (oe_id, msg);
            meter.event (this);
        }
    }

    inline void quote (price_t bid_, price_t ask_)
    {
        assert (false);
    }

    inline void throughput (uint8_t meter_id_, uint64_t frequency_)
    {
        assert (false);
    }

    inline void frequency (uint8_t meter_id_, uint64_t frequency_)
    {
        //  Send the throughput figure to the stat component
        zmq::message_t msg;
        make_throughput (meter_id_, frequency_, &msg);
        api->send (se_id, msg);
    }

private:

    //  0MQ infrastructure
    zmq::api_thread_t *api;
    zmq::poll_thread_t *pt;

    //  Exchange IDs
    int oe_id;
    int se_id;

    //  Measuring the rate of outgoing messages (orders)
    frequency_meter_t meter;
};

//  Receiver half of the gateway application

class receiver_t
{
public:

    receiver_t (zmq::dispatcher_t *dispatcher_, zmq::locator_t *locator_) :
        meter (500000, 4),
        last_timestamp (0)
    {
        //  Initialise 0MQ infrastructure
        api = zmq::api_thread_t::create (dispatcher_, locator_);
        pt = zmq::poll_thread_t::create (dispatcher_);

        //  Initialise the wiring
        api->create_queue ("TQ");
        api->bind ("TE", "TQ", pt, pt);
        se_id = api->create_exchange ("SE");
        api->bind ("SE", "SQ", pt, pt);
    }

    void run ()
    {
        //  Main message dispatch loop
        while (true) {
            zmq::message_t msg;
            api->receive (&msg);
            parse_message (&msg, this);
        }
    }

    inline void order (order_id_t order_id_, order_type_t type_,
        price_t price_, volume_t volume_)
    {
        assert (false);
    }

    inline void order_confirmation (order_id_t order_id_)
    {
        //  Measuring throughput
        meter.event (this);

        //  Taking timestamps
        if (order_id_ % 500000 == 0 && order_id_ > last_timestamp) {
            zmq::message_t msg;
            make_timestamp (6, order_id_, perf::now () / 1000, &msg);
            api->send (se_id, msg);
            last_timestamp = order_id_;
        }
    }

    inline void trade (order_id_t order_id_, price_t price_, volume_t volume_)
    {
        //  As we are doing to business logic with trades and
        //  order confirmations we can handle them in the same manner.
        order_confirmation (order_id_);
    }

    inline void quote (price_t bid_, price_t ask_)
    {
        //  Measuring throughput
        meter.event (this);
    }

    inline void throughput (uint8_t meter_id_, uint64_t frequency_)
    {
        assert (false);
    }

    inline void timestamp (uint8_t meter_id_, uint64_t correlation_id_,
        uint64_t frequency_)
    {
        assert (false);
    }

    inline void frequency (uint8_t meter_id_, uint64_t frequency_)
    {
        //  Send the throughput figure to the stat component
        zmq::message_t msg;
        make_throughput (meter_id_, frequency_, &msg);
        api->send (se_id, msg);
    }

private:

    //  0MQ infrastructure
    zmq::api_thread_t *api;
    zmq::poll_thread_t *pt;

    //  Exchange IDs
    int se_id;

    //  Measuring the rate of incoming messages (trades & order confirmations)
    frequency_meter_t meter;

    //  Order ID for which last timestamp was taken
    order_id_t last_timestamp;
};

//  Arguments to be passed to the sender_routine
struct sender_routine_args_t
{
    zmq::dispatcher_t *dispatcher;
    zmq::locator_t *locator;
    int order_rate;
};

//  Main routine for the sender thread
void *sender_routine (void *arg_)
{
    //  Start the sender with the rate of X orders per second
    sender_routine_args_t *args = (sender_routine_args_t*) arg_;
    sender_t sender (args->dispatcher, args->locator);
    sender.run (args->order_rate);
    return NULL;
}

int main (int argc, char *argv [])
{
    if (argc != 3) {
        printf ("Usage: gtw <hostname> <orders per second>\n");
        return 1;
    }

    //  Create the shared message dispatcher
    zmq::dispatcher_t dispatcher (4);

    //  Create a resource locator
    zmq::locator_t locator (argv [1]);

    //  Run the sender thread
    pthread_t sender_thread;
    sender_routine_args_t args = {&dispatcher, &locator, atoi (argv [2])};
    int rc = pthread_create (&sender_thread, NULL, sender_routine, &args);
    assert (rc == 0);

    //  Run the receiving loop
    receiver_t receiver (&dispatcher, &locator);
    receiver.run ();

    return 0;
}
