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

#ifndef __ZMQ_BP_DECODER_HPP_INCLUDED__
#define __ZMQ_BP_DECODER_HPP_INCLUDED__

#include "demux.hpp"
#include "decoder.hpp"
#include "message.hpp"

namespace zmq
{
    //  Decoder for 0MQ backend protocol. Converts data batches into messages.

    class bp_decoder_t : public decoder_t <bp_decoder_t>
    {
    public:

        bp_decoder_t (demux_t *demux_);

    protected:

        void one_byte_size_ready ();
        void eight_byte_size_ready ();
        void message_ready ();

        demux_t *demux;
        unsigned char tmpbuf [8];
        message_t message;
    };

}

#endif
