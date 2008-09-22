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

#include <string.h>
#include <string>
#include <iostream>

using namespace std;

#include "../../zmq/dispatcher.hpp"
#include "../../zmq/locator.hpp"
#include "../../zmq/poll_thread.hpp"
#include "../../zmq/api_thread.hpp"
#include "../../zmq/message.hpp"

using namespace zmq;

int main (int argc, const char *argv [])
{
    //  Check the command line syntax.
    if (argc != 3) {
        cerr << "usage: display <host> <chatroom name>" << endl;
        return 1;
    }

    //  Retrieve command line arguments
    const char *host = argv [1];
    const char *chatroom_name = argv [2];

    //  Initialise 0MQ infrastructure.
    dispatcher_t dispatcher (2);
    locator_t locator (host);

    //  Initialise the thread layout
    poll_thread_t *pt = poll_thread_t::create (&dispatcher);
    api_thread_t *api = api_thread_t::create (&dispatcher, &locator);

    //  Create local queue to receive messages.
    api->create_queue ("Q");

    //  Bind the queue to globally accessible exchange exposed by chatroom.
    //  If the exchange is not available, assert.
    char tmp [16];
    snprintf (tmp, 16, "E_%s", chatroom_name);
    api->bind (tmp, "Q", pt, pt);
    
    while (true) {

        //  Get a message and print it to the console.
        message_t message;
        api->receive (&message);
        cout << (char*) message.data ();
    }
}
