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

#ifndef __ZMQ_BP_ENCODER_HPP_INCLUDED__
#define __ZMQ_BP_ENCODER_HPP_INCLUDED__

#include <stddef.h>
#include <assert.h>

#include "mux.hpp"
#include "encoder.hpp"
#include "message.hpp"

namespace zmq
{
    //  Encoder for 0MQ backend protocol. Converts messages into data batches.

    class bp_encoder_t : public encoder_t <bp_encoder_t>
    {
    public:

        bp_encoder_t (mux_t *mux_);
        ~bp_encoder_t ();

    protected:

        bool size_ready ();
        bool message_ready ();

        mux_t *mux;
        message_t message;
        unsigned char tmpbuf [9];
    };

}

#endif
