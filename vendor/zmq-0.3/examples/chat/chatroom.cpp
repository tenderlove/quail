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

#include <time.h>
#include <string.h>
#include <iostream>

using namespace std;

#include "../../zmq/dispatcher.hpp"
#include "../../zmq/locator.hpp"
#include "../../zmq/poll_thread.hpp"
#include "../../zmq/api_thread.hpp"
#include "../../zmq/message.hpp"

using namespace zmq;

bool error_handler (const char*)
{
    //  We don't want chatroom to fail when clent disconnects
    return true;
}

int main (int argc, const char *argv [])
{
    //  Check the command line syntax
    if (argc != 5) {
        cerr << "usage: chatroom <hostname> <chatroom name> "
            "<in interface> <out interface>" << endl;
        return 1;
    }

    //  Retrieve command line arguments
    const char *host = argv [1];
    const char *chatroom_name = argv [2];
    const char *in_interface = argv [3];
    const char *out_interface = argv [4];

    //  Initialise 0MQ infrastructure
    set_error_handler (error_handler);
    dispatcher_t dispatcher (2);
    locator_t locator (host);

    //  Initialise the thread layout
    poll_thread_t *pt = poll_thread_t::create (&dispatcher);
    api_thread_t *api = api_thread_t::create (&dispatcher, &locator);

    //  Create a queue to receive messages sent to the chatroom
    char tmp [16];
    snprintf (tmp, 16, "Q_%s", chatroom_name);
    api->create_queue (tmp, scope_global, in_interface, pt, 1, &pt);

    //  Create an exchange to send messages from the chatroom
    snprintf (tmp, 16, "E_%s", chatroom_name);
    int eid = api->create_exchange (tmp, scope_global, out_interface,
        pt, 1, &pt);

    while (true) {

        //  Get a message
        message_t in_message;
        api->receive (&in_message);

        //  Get the current time. Replace the newline character at the end
        //  by space character.
        char timebuf [256];
        time_t current_time;
        time (&current_time);
        snprintf (timebuf, 256, ctime (&current_time));
        timebuf [strlen (timebuf) - 1] = ' ';

        //  Create and fill in the message
        message_t out_message (strlen (timebuf) + in_message.size ());
        char *data = (char*) out_message.data ();
        memcpy (data, timebuf, strlen (timebuf));
        data += strlen (timebuf);
        memcpy (data, in_message.data (), in_message.size ());

        //  Send the message
        api->send (eid, out_message);
    }
}
