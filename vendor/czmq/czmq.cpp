
#include "czmq.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <zmq/message.hpp>
#include <zmq/locator.hpp>
#include <zmq/dispatcher.hpp>
#include <zmq/poll_thread.hpp>
#include <zmq/api_thread.hpp>

struct context_t
{
    zmq::locator_t *locator;
    zmq::dispatcher_t *dispatcher;
    zmq::poll_thread_t *poll_thread;
    zmq::api_thread_t *api_thread;
};

void *czmq_create (const char *host_)
{
    //  Create the context.
    context_t *context = new context_t;
    assert (context);
    context->locator = new zmq::locator_t (host_);
    assert (context->locator);
    context->dispatcher = new zmq::dispatcher_t (2);
    assert (context->dispatcher);
    context->poll_thread = zmq::poll_thread_t::create (context->dispatcher);
    assert (context->poll_thread);
    context->api_thread = zmq::api_thread_t::create (context->dispatcher,
        context->locator);
    assert (context->api_thread);

    return (void*) context;
}

void czmq_destroy (void *obj_)
{
    //  Get the context.
    context_t *context = (context_t*) obj_;

    //  Deallocate the 0MQ infrastructure.
    delete context->dispatcher;
    delete context->locator;
    delete context;
}

int czmq_create_exchange (void *obj_, const char *exchange_, int scope_,
    const char *nic_)
{
    //  Get the context.
    context_t *context = (context_t*) obj_;

    //  Get the scope.
    zmq::scope_t scope = zmq::scope_local;
    if (scope_ == CZMQ_SCOPE_GLOBAL)
        scope = zmq::scope_global;

    //  Forward the call to native 0MQ library.
    return context->api_thread->create_exchange (exchange_, scope, nic_,
        context->poll_thread, 1, &context->poll_thread);
}

int czmq_create_queue (void *obj_, const char *queue_, int scope_,
    const char *nic_)
{
    //  Get the context.
    context_t *context = (context_t*) obj_;

    //  Get the scope.
    zmq::scope_t scope = zmq::scope_local;
    if (scope_ == CZMQ_SCOPE_GLOBAL)
        scope = zmq::scope_global;

    //  Forward the call to native 0MQ library.
    return context->api_thread->create_queue (queue_, scope, nic_,
        context->poll_thread, 1, &context->poll_thread);
}

void czmq_bind (void *obj_, const char *exchange_, const char *queue_)
{
    //  Get the context.
    context_t *context = (context_t*) obj_;

    //  Forward the call to native 0MQ library.
    context->api_thread->bind (exchange_, queue_,
        context->poll_thread, context->poll_thread);
}

void czmq_send (void *obj_, int eid_, void *data_, size_t size_,
    czmq_free_fn *ffn_)
{
    //  Get the context.
    context_t *context = (context_t*) obj_;

    //  Forward the call to native 0MQ library.
    zmq::message_t msg (data_, size_, ffn_);
    context->api_thread->send (eid_, msg);
}

void czmq_receive (void *obj_, void **data_, size_t *size_, czmq_free_fn **ffn_)
{
    //  Get the context.
    context_t *context = (context_t*) obj_;

    //  Forward the call to native 0MQ library.
    zmq::message_t msg;
    context->api_thread->receive (&msg);

    //  Create a buffer and copy the data into it.
    void *buf = malloc (msg.size ());
    assert (buf);
    memcpy (buf, msg.data (), msg.size ());

    //  Return the message.
    *data_ = buf;
    *size_ = msg.size ();
    *ffn_ = free;   
}
