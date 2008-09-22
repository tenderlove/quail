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

//  Video over 0MQ, sender half.

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

#include <unicap.h>
#include "./ucil.h"

#include <zmq/dispatcher.hpp>
#include <zmq/api_thread.hpp>
#include <zmq/poll_thread.hpp>
#include <zmq/locator.hpp>
#include <zmq/message.hpp>
#include <zmq/wire.hpp>

#define FOURCC(a,b,c,d) (unsigned int)((((unsigned int)d)<<24)+\
    (((unsigned int)c)<<16)+(((unsigned int)b)<<8)+a)

bool error_handler (const char*)
{
    //  We don't want sender to fail when receiver disconnects
    return true;
}

int main (int argc, char *argv [])
{
    unicap_handle_t handle;
    unicap_device_t device;
    unicap_format_t src_format;
    unicap_format_t dest_format;
    unicap_data_buffer_t src_buffer;
    unicap_data_buffer_t dest_buffer;
    unicap_data_buffer_t *returned_buffer;

    if (argc != 4) {
        fprintf (stderr, "Usage: sender <hostname> <camera name> "
            "<interface>\n");
        exit (1);
    }

    //  Initialise 0MQ infrastructure

    //  1. Set error handler function (to ignore disconnected receivers)
    zmq::set_error_handler (error_handler);

    //  2. Initialise basic infrastructure for 2 threads
    zmq::dispatcher_t dispatcher (2);

    //  3. Initialise local locator (to connect to global locator)
    zmq::locator_t locator (argv [1]);

    //  4. Start one working thread (to send data to receivers)
    zmq::poll_thread_t *pt = zmq::poll_thread_t::create (&dispatcher);

    //  5. Register one API thread (the application thread - the one that
    //     is being executed at the moment)
    zmq::api_thread_t *api = zmq::api_thread_t::create (&dispatcher, &locator);

    //  6.  Define an entry point for the messages. The name of the entry point
    //      is user-defined ("camera name"). Specify that working thread "pt"
    //      will be used to listen to new connections being created as well as
    //      to send frames to existing connections.
    int e_id = api->create_exchange (argv [2], zmq::scope_global, argv [3],
        pt, 1, &pt);
    
    //  Open first available video capture device
    if (!SUCCESS (unicap_enumerate_devices (NULL, &device, 0))) {
        fprintf (stderr, "Could not enumerate devices\n");
        exit (1);
    }
    if (!SUCCESS (unicap_open (&handle, &device))) {
        fprintf (stderr, "Failed to open device: %s\n", device.identifier);
        exit (1);
    }
    printf( "Opened video capture device: %s\n", device.identifier );

    //  Find a suitable video format that we can convert to RGB24
    bool conversion_found = false;
    int index = 0;
    while (SUCCESS (unicap_enumerate_formats (handle, NULL, &src_format,
          index))) {
        printf ("Trying video format: %s\n", src_format.identifier);
        if (ucil_conversion_supported (FOURCC ('R', 'G', 'B', '3'), 
            src_format.fourcc)) {
            conversion_found = true;
            break;
        }
        index++;
    }
    if (!conversion_found) {
        fprintf (stderr, "Could not find a suitable video format\n");
        exit (1);
    }
    src_format.buffer_type = UNICAP_BUFFER_TYPE_USER;
    if (!SUCCESS (unicap_set_format (handle, &src_format))) {
        fprintf (stderr, "Failed to set video format\n");
        exit (1);
    }
    printf ("Using video format: %s [%dx%d]\n", 
        src_format.identifier, 
        src_format.size.width, 
        src_format.size.height);

    //  Clone destination format with equal dimensions, but RGB24 colorspace
    unicap_copy_format (&dest_format, &src_format);
    strcpy (dest_format.identifier, "RGB 24bpp");
    dest_format.fourcc = FOURCC ('R', 'G', 'B', '3');
    dest_format.bpp = 24;
    dest_format.buffer_size = dest_format.size.width *
        dest_format.size.height * 3;
    
    //  Initialise image buffers
    memset (&src_buffer, 0, sizeof (unicap_data_buffer_t));
    src_buffer.data = (unsigned char *)malloc (src_format.buffer_size);
    src_buffer.buffer_size = src_format.buffer_size;
    memset (&dest_buffer, 0, sizeof (unicap_data_buffer_t));
    dest_buffer.data = (unsigned char *)malloc (dest_format.buffer_size);
    dest_buffer.buffer_size = dest_format.buffer_size;
    dest_buffer.format = dest_format;

    //  Start video capture
    if (!SUCCESS (unicap_start_capture (handle))) {
        fprintf (stderr, "Failed to start capture on device: %s\n",
            device.identifier);
        exit (1);
    }

    //  Loop, sending video to defined exchange
    while (1) {

        //  Queue buffer for video capture
        if (!SUCCESS (unicap_queue_buffer (handle, &src_buffer))) {
            fprintf (stderr, "Failed to queue a buffer on device: %s\n",
                device.identifier);
            exit (1);
        }

        //  Wait until buffer is ready
        if (!SUCCESS (unicap_wait_buffer (handle, &returned_buffer))) {
            fprintf (stderr, "Failed to wait for buffer on device: %s\n",
                device.identifier);
            exit (1);
        }

        //  Convert colorspace
        if (!SUCCESS (ucil_convert_buffer (&dest_buffer, &src_buffer))) {
            //  TODO: This fails sometimes for unknown reasons,
            //  just skip the frame for now
            fprintf (stderr, "Failed to convert video buffer\n");
        }

        //  Create ZMQ message
        zmq::message_t msg (dest_format.buffer_size + (2 * sizeof (uint32_t)));
        unsigned char *data = (unsigned char *)msg.data();

        //  Image width in pixels
        zmq::put_uint32 (data, (uint32_t)dest_format.size.width);
        data += sizeof (uint32_t);

        //  Image height in pixels
        zmq::put_uint32 (data, (uint32_t)dest_format.size.height);
        data += sizeof (uint32_t);

        //  RGB24 image data
        memcpy (data, dest_buffer.data, dest_format.buffer_size);

        //  Send message 
        api->send (e_id, msg);
    }
    
    return 0;
}
