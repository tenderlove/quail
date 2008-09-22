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
#include "../../perf/helpers/time.hpp"

#include "messages.hpp"
#include "frequency_meter.hpp"
#include "matching_engine.hpp"
using namespace exchange;

//  Matching engine application

class me_t
{
public:

    inline me_t (const char *host_, const char *in_interface_,
          const char *out_interface_) :
        dispatcher (3),
        locator (host_),
	in_meter (500000, 2),
	out_meter (500000, 3)
    {
        //  Initialise 0MQ infrastructure
        api = zmq::api_thread_t::create (&dispatcher, &locator);
        pt_in = zmq::poll_thread_t::create (&dispatcher);
        pt_out = zmq::poll_thread_t::create (&dispatcher);

        //  Initialise the wiring
        te_id = api->create_exchange ("TE", zmq::scope_global,
            out_interface_, pt_out, 1, &pt_out);
        api->create_queue ("OQ", zmq::scope_global,
            in_interface_, pt_in, 1, &pt_in);
        se_id = api->create_exchange ("SE");
        api->bind ("SE", "SQ", pt_out, pt_out);
    }

    void run ()
    {
        //  Message dispatch loop
        while (true) {
            zmq::message_t msg;
            api->receive (&msg);
            parse_message (&msg, this);
        }
    }

    inline void order (order_id_t order_id_, order_type_t type_,
        price_t price_, volume_t volume_)
    {
        in_meter.event (this);

        //  Pass the order to matching engine
	bool trades_sent;
        if (type_ == ask)
            trades_sent = me.ask (this, order_id_, price_, volume_);
        else
            trades_sent = me.bid (this, order_id_, price_, volume_);

	//  If no trade was executed, send order confirmation
	if (!trades_sent) {
            zmq::message_t msg;
	    make_order_confirmation (order_id_, &msg);
	    api->presend (te_id, msg);
	    out_meter.event (this);
	}

	//  Flush the outgoing messages
        api->flush ();
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
        assert (false);
    }

    inline void timestamp (uint8_t meter_id_, uint64_t correlation_id_,
        uint64_t timestamp_)
    {
        assert (false);
    }

    inline void traded (order_id_t order_id_, price_t price_, volume_t volume_)
    {
        //  Send trade back to the gateway
        zmq::message_t msg;
        make_trade (order_id_, price_, volume_, &msg);
        api->presend (te_id, msg);
	out_meter.event (this);
    }

    inline void quoted (price_t bid_, price_t ask_)
    {
        //  Send quote back to the gateway
        zmq::message_t msg;
        make_quote (ask_, bid_, &msg);
        api->presend (te_id, msg);
	out_meter.event (this);
    }

    inline void frequency (uint8_t meter_id_, uint64_t frequency_)
    {
        //  Send the throughput figure to the stat component
        zmq::message_t msg;
        make_throughput (meter_id_, frequency_, &msg);
        api->presend (se_id, msg);
    }

private:

   matching_engine_t me;
   zmq::dispatcher_t dispatcher;
   zmq::locator_t locator;
   zmq::api_thread_t *api;
   zmq::poll_thread_t *pt_in;
   zmq::poll_thread_t *pt_out;
   int te_id;
   int se_id;
   frequency_meter_t in_meter;
   frequency_meter_t out_meter;
};

int main (int argc, char *argv [])
{
    if (argc != 4) {
        printf ("Usage: me <hostname> <in interface> <out interface>\n");
        return 1;
    }

    //  Run the matching engine
    me_t me (argv [1], argv [2], argv [3]);
    me.run (); 
    return 0;
}
