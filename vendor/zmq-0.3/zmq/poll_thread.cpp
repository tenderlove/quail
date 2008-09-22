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

#include <algorithm>

#include "poll_thread.hpp"
#include "err.hpp"

zmq::poll_thread_t *zmq::poll_thread_t::create (dispatcher_t *dispatcher_)
{
    return new poll_thread_t (dispatcher_);
}

zmq::poll_thread_t::poll_thread_t (dispatcher_t *dispatcher_) :
    dispatcher (dispatcher_),
    pollset (1)
{
    //  Initialise the pollset.
    pollset [0].fd = signaler.get_fd ();
    pollset [0].events = POLLIN;

    //  Register the thread with command dispatcher.
    thread_id = dispatcher->allocate_thread_id (&signaler);

    //  Create the worker thread.
    int rc = pthread_create (&worker, NULL, worker_routine, this);
    errno_assert (rc == 0);
}

zmq::poll_thread_t::~poll_thread_t ()
{
    //  Send a 'stop' event ot the worker thread.
    //  TODO: Analyse whether using the to-self command pipe here is appropriate
    command_t cmd;
    cmd.init_stop ();
    dispatcher->write (thread_id, thread_id, cmd);

    //  Wait till worker thread terminates.
    int rc = pthread_join (worker, NULL);
    errno_assert (rc == 0);
}

void zmq::poll_thread_t::register_engine (i_pollable *engine_)
{
    //  Plug the engine to the poll thread (via admin context).
    command_t command;
    command.init_register_engine (engine_);
    dispatcher->send_command (this, command);
}

int zmq::poll_thread_t::get_thread_id ()
{
    return thread_id;
}

void zmq::poll_thread_t::send_command (i_context *destination_,
    const command_t &command_)
{
    dispatcher->write (thread_id, destination_->get_thread_id (), command_);
}

void *zmq::poll_thread_t::worker_routine (void *arg_)
{
    poll_thread_t *self = (poll_thread_t*) arg_;
    self->loop ();
    return 0;
}

void zmq::poll_thread_t::loop ()
{
    while (true)
    {
        //  Adjust the events to wait - the engine chooses the events.
        for (engines_t::size_type engine_nbr = 0;
              engine_nbr != engines.size (); engine_nbr ++)
            pollset [engine_nbr + 1].events =
                engines [engine_nbr]->get_events ();

        //  TODO: Polling all the time isn't efficient. We should do it in
        //  a manner similar to API thread. I.e. loop throught dispatcher
        //  if messages are available and and poll only occasinally (every
        //  100 messages or so). If there are no messages, poll immediately.

        //  Wait for events.
        int rc = poll (&pollset [0], pollset.size (), -1);
        errno_assert (rc != -1);

        //  First of all, process socket errors.
        for (pollset_t::size_type pollset_index = 1;
              pollset_index != pollset.size (); pollset_index ++) {
            if (pollset [pollset_index].revents &
                  (POLLNVAL | POLLERR | POLLHUP)) {
                engines [pollset_index - 1]->close_event ();
                unregister_engine (engines[pollset_index - 1]);
                pollset_index --;
            }
        }

        //  Process commands from other threads.
        if (pollset [0].revents & POLLIN) {
            uint32_t signals = signaler.check ();
            assert (signals);
            if (!process_commands (signals))
                return;
        }

        //  Process out events from the engines.
        for (pollset_t::size_type pollset_index = 1;
              pollset_index != pollset.size (); pollset_index ++) {
            if (pollset [pollset_index].revents & POLLOUT) {
                if (!engines [pollset_index - 1]->out_event ()) {
                    engines [pollset_index - 1]->close_event ();
                    unregister_engine (engines[pollset_index - 1]);
                    pollset_index --;
                }
            }
        }

        //  Process in events from the engines.
        for (pollset_t::size_type pollset_index = 1;
              pollset_index != pollset.size (); pollset_index ++) {

            //  TODO: investigate the POLLHUP issue on OS X
            if (pollset [pollset_index].revents & (POLLIN /*| POLLHUP*/))
                if (!engines [pollset_index - 1]->in_event ()) {
                    engines [pollset_index - 1]->close_event ();
                    unregister_engine (engines[pollset_index - 1]);
                    pollset_index--;
                }
        }
    }
}

void zmq::poll_thread_t::unregister_engine (i_pollable* engine_)
{
    //  Find the engine in the list.
    std::vector <i_pollable*>::iterator it =std::find (
        engines.begin (), engines.end (),
        engine_);
    assert (it != engines.end ());

    //  Remove the engine from the engine list and the pollset.
    int pos = it - engines.begin ();
    engines.erase (it);
    pollset.erase (pollset.begin () + 1 + pos);

    // TODO: delete engine_;
}

bool zmq::poll_thread_t::process_commands (uint32_t signals_)
{
    //  Iterate through all the threads in the process and find out which
    //  of them sent us commands.
    for (int source_thread_id = 0;
          source_thread_id != dispatcher->get_thread_count ();
          source_thread_id ++) {
        if (signals_ & (1 << source_thread_id)) {

            //  Read all the commands from particular thread.
            command_t command;
            while (dispatcher->read (source_thread_id, thread_id, &command)) {

                switch (command.type) {

                //  Exit the working thread.
                case command_t::stop:
                    return false;

                //  Register the engine supplied with the poll thread.
                case command_t::register_engine:
                    {
                        //  Add the engine to the engine list.
                        i_pollable *engine =
                            command.args.register_engine.engine;

                        //  Store the engine pointer.
                        engines.push_back (engine);

                        //  Add the engine to the pollset.
                        pollfd pfd = {engine->get_fd (),
                            engine->get_events (), 0};
                        pollset.push_back (pfd);
                    }
                    break;

                //  Unregister the engine.
                case command_t::unregister_engine:
                    {
                        //  Find the engine in the list.
                        std::vector <i_pollable*>::iterator it =std::find (
                            engines.begin (), engines.end (),
                            command.args.unregister_engine.engine);
                        assert (it != engines.end ());

                        //  Remove the engine from the engine list and
                        //  the pollset.
                        int pos = it - engines.begin ();
                        engines.erase (it);
                        pollset.erase (pollset.begin () + 1 + pos);
                    }
                    break;


                //  Forward the command to the specified engine.
                case command_t::engine_command:

                    //  TODO: check whether the engine still exists,
                    //  Otherwise drop the command.
                    command.args.engine_command.engine->process_command (
                        command.args.engine_command.command);
                    break;

                //  Unknown command.
                default:

                    assert (false);
                }
            }
        }
    }
    return true;
}
