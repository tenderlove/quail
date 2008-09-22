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

#include <poll.h>

#include "bp_engine.hpp"

zmq::bp_engine_t *zmq::bp_engine_t::create (poll_thread_t *thread_,
    const char *host_, size_t writebuf_size_,
    size_t readbuf_size_, const char *local_object_)
{
    bp_engine_t *instance = new bp_engine_t (
        thread_, host_, writebuf_size_, readbuf_size_, local_object_);
    assert (instance);

    return instance;
}

zmq::bp_engine_t *zmq::bp_engine_t::create (poll_thread_t *thread_,
    tcp_listener_t &listener_, size_t writebuf_size_, size_t readbuf_size_,
    const char *local_object_)
{
    bp_engine_t *instance = new bp_engine_t (
        thread_, listener_, writebuf_size_, readbuf_size_, local_object_);
    assert (instance);

    return instance;
}

zmq::bp_engine_t::bp_engine_t (poll_thread_t *thread_, const char *host_,
      size_t writebuf_size_, size_t readbuf_size_,
      const char *local_object_) :
    context (thread_),
    writebuf_size (writebuf_size_),
    readbuf_size (readbuf_size_),
    write_size (0),
    write_pos (0),
    encoder (&mux),
    decoder (&demux),
    socket (host_, NULL, NULL),
    events (POLLIN),
    socket_error (false),
    local_object (local_object_)
{
    //  Allocate read and write buffers.
    writebuf = (unsigned char*) malloc (writebuf_size);
    assert (writebuf);
    readbuf = (unsigned char*) malloc (readbuf_size);
    assert (readbuf);

    //  Register BP engine with the I/O thread.
    thread_->register_engine (this);
}

zmq::bp_engine_t::bp_engine_t (poll_thread_t *thread_,
      tcp_listener_t &listener_, size_t writebuf_size_, size_t readbuf_size_,
      const char *local_object_) :
    context (thread_),
    writebuf_size (writebuf_size_),
    readbuf_size (readbuf_size_),
    write_size (0),
    write_pos (0),
    encoder (&mux),
    decoder (&demux),
    socket (listener_),
    events (POLLIN),
    socket_error (false),
    local_object (local_object_)
{
    //  Allocate read and write buffers.
    writebuf = (unsigned char*) malloc (writebuf_size);
    assert (writebuf);
    readbuf = (unsigned char*) malloc (readbuf_size);
    assert (readbuf);

    //  Register BP engine with the I/O thread.
    thread_->register_engine (this);
}

zmq::bp_engine_t::~bp_engine_t ()
{
    free (readbuf);
    free (writebuf);
}

int zmq::bp_engine_t::get_fd ()
{
    return socket.get_fd ();
}

short zmq::bp_engine_t::get_events ()
{
    //  TODO:
    //  return events | (proxy.has_messages () ? POLLOUT : 0);
    return events;
}

bool zmq::bp_engine_t::in_event ()
{
    //  Read as much data as possible to the read buffer.
    size_t nbytes = socket.read (readbuf, readbuf_size);

    if (!nbytes) {

        //  If the other party closed the connection, stop polling.
        //  TODO: handle the event more gracefully
        events ^= POLLIN;
        return false;
    }

    //  Push the data to the decoder
    decoder.write (readbuf, nbytes);

    //  Flush any messages decoder may have produced.
    demux.flush ();

    return true;
}

bool zmq::bp_engine_t::out_event ()
{
    //  If write buffer is empty, try to read new data from the encoder.
    if (write_pos == write_size) {

        write_size = encoder.read (writebuf, writebuf_size);
        write_pos = 0;

        //  If there are no data to write stop polling for output.
        if (!write_size)
            events ^= POLLOUT;
    }

    //  If there are any data to write in write buffer, write as much as
    //  possible to the socket.
    if (write_pos < write_size) {
        ssize_t nbytes = (ssize_t) socket.write (writebuf + write_pos,
            write_size - write_pos);
        if (nbytes <= 0) 
            return false;
        write_pos += nbytes;
    }
    return true;
}

void zmq::bp_engine_t::close_event ()
{
    if (!socket_error) {
        socket_error = true;

        //  Report connection failure to the client.
        //  If there is no error handler, application crashes immediately.
        //  If the error handler returns false, it crashes as well.
        //  If error handler returns true, the error is ignored.       
        error_handler_t *eh = get_error_handler ();
        assert (eh);
        if (!eh (local_object.c_str ()))
            assert (false);

        //  Notify all our receivers that this engine is shutting down.
        demux.terminate_pipes ();

        //  Notify senders that this engine is shutting down.
        mux.terminate_pipes ();
    }
}

void zmq::bp_engine_t::process_command (const engine_command_t &command_)
{
    switch (command_.type) {
    case engine_command_t::revive:

        //  Forward the revive command to the pipe.
        if (!socket_error)
            command_.args.revive.pipe->revive ();

        //  There is at least one engine that has messages ready -
        //  start polling the socket for writing.
        events |= POLLOUT;
        break;

    case engine_command_t::send_to:

        //  Start sending messages to a pipe.
        if (!socket_error)
            demux.send_to (command_.args.send_to.pipe);
        break;

    case engine_command_t::receive_from:

        //  Start receiving messages from a pipe.
        if (!socket_error) {
            mux.receive_from (command_.args.receive_from.pipe);
            events |= POLLOUT;
        }
        break;

    case engine_command_t::destroy_pipe:
        demux.destroy_pipe (command_.args.destroy_pipe.pipe);
        delete command_.args.destroy_pipe.pipe;
        break;

    default:
        assert (false);
    }
}
