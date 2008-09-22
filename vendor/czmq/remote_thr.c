
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "czmq.h"

int main (int argc, char *argv [])
{
    const char *host;
    const char *in_interface;
    const char *out_interface;
    int message_size;
    int message_count;
    void *handle;
    int eid;
    int counter;
    void *out_buf;
    size_t size;
    czmq_free_fn *ffn;

    //  Parse command line arguments.
    if (argc != 4) {
        printf ("usage: remote_thr <hostname> <message-size> "
            "<message-count>\n");
        return 1;
    }
    host = argv [1];
    message_size = atoi (argv [2]);
    message_count = atoi (argv [3]);

    //  Create 0MQ transport.
    handle = czmq_create (host);

    //  Create the wiring.
    eid = czmq_create_exchange (handle, "E", CZMQ_SCOPE_LOCAL, NULL);
    czmq_bind (handle, "E", "Q");

    //  Create message data to send.
    out_buf = malloc (message_size);
    assert (out_buf);

    for (counter = 0; counter != message_count + 1; counter ++)
        czmq_send (handle, eid, out_buf, message_size, NULL);

    //  Wait till all messages are sent.
    sleep (3600);

    //  Destroy 0MQ transport.
    czmq_destroy (handle);

    //  Clean up.
    free (out_buf);

    return 0;
}
