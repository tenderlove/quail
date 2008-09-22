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

#ifndef __EXCHANGE_MESSAGES_HPP_INCLUDED__
#define __EXCHANGE_MESSAGES_HPP_INCLUDED__

#include "../../zmq/message.hpp"
#include "../../zmq/wire.hpp"

#include "common.hpp"

namespace exchange
{

    enum order_type_t
    {
        ask = 1,
        bid = 2
    };

    enum message_type_t
    {
        //  Business messages
        msg_type_order = 1,
        msg_type_order_confirmation = 2,
        msg_type_trade = 3,
        msg_type_quote = 4,

        //  Instrumentation messages
        msg_type_throughput = 5,
        msg_type_timestamp = 6
    };

    //  Creates an 'order' message
    void make_order (order_id_t order_id_, order_type_t type_,
        price_t price_, volume_t volume_, zmq::message_t *msg_)
    {
        msg_->rebuild (10);
        unsigned char *buff = (unsigned char*) msg_->data ();
        zmq::put_uint8 (buff, msg_type_order);
        buff += sizeof (uint8_t);
        zmq::put_uint32 (buff, order_id_);
        buff += sizeof (uint32_t);
        zmq::put_uint8 (buff, type_);
        buff += sizeof (uint8_t);
        zmq::put_uint16 (buff, price_);
        buff += sizeof (uint16_t);
        zmq::put_uint16 (buff, volume_);
    }

    //  Creates an 'order confirmation' message
    void make_order_confirmation (order_id_t order_id_, zmq::message_t *msg_)
    {
        msg_->rebuild (5);
        unsigned char *buff = (unsigned char*) msg_->data ();
        zmq::put_uint8 (buff, msg_type_order_confirmation);
        buff += sizeof (uint8_t);
        zmq::put_uint32 (buff, order_id_);
    }

    //  Create 'trade' message
    void make_trade (order_id_t order_id_, price_t price_, volume_t volume_,
        zmq::message_t *msg_)
    {
        msg_->rebuild (9);
        unsigned char *buff = (unsigned char*) msg_->data ();
        zmq::put_uint8 (buff, msg_type_trade);
        buff += sizeof (uint8_t);
        zmq::put_uint32 (buff, order_id_);
        buff += sizeof (uint32_t);
        zmq::put_uint16 (buff, price_);
        buff += sizeof (uint16_t);
        zmq::put_uint16 (buff, volume_);
    }

    //  Create 'quote' message
    void make_quote (price_t bid_, price_t ask_, zmq::message_t *msg_)
    {
        msg_->rebuild (5);
        unsigned char *buff = (unsigned char*) msg_->data ();
        zmq::put_uint8 (buff, msg_type_quote);
        buff += sizeof (uint8_t);
        zmq::put_uint16 (buff, bid_);
        buff += sizeof (uint16_t);
        zmq::put_uint16 (buff, ask_);
    }

    //  Create 'throughput' message
    void make_throughput (uint8_t meter_id_, uint64_t throughput_,
        zmq::message_t *msg_)
    {
        msg_->rebuild (10);
        unsigned char *buff = (unsigned char*) msg_->data ();
        zmq::put_uint8 (buff, msg_type_throughput);
        buff += sizeof (uint8_t);
        zmq::put_uint8 (buff, meter_id_);
        buff += sizeof (uint8_t);
        zmq::put_uint64 (buff, throughput_);
    }

    //  Create 'timestamp' message
    void make_timestamp (uint8_t meter_id_, uint64_t correlation_id_,
        uint64_t timestamp_, zmq::message_t *msg_)
    {
        msg_->rebuild (18);
        unsigned char *buff = (unsigned char*) msg_->data ();
        zmq::put_uint8 (buff, msg_type_timestamp);
        buff += sizeof (uint8_t);
        zmq::put_uint8 (buff, meter_id_);
        buff += sizeof (uint8_t);
        zmq::put_uint64 (buff, correlation_id_);
        buff += sizeof (uint64_t);
        zmq::put_uint64 (buff, timestamp_);
    }

    //  Parse a message and call the appropriate function the callback object
    template <typename T> void parse_message (zmq::message_t *msg_,
        T *callback_)
    {
        unsigned char *buff = (unsigned char*) msg_->data ();
        size_t size = msg_->size ();

        assert (size >= sizeof (uint8_t));
        message_type_t type = (message_type_t) zmq::get_uint8 (buff);
        buff += sizeof (uint8_t);
        size -= sizeof (uint8_t);

        switch (type) {
        case msg_type_order:
            {
                assert (size == 9);
                order_id_t order_id = zmq::get_uint32 (buff);
                buff += sizeof (uint32_t);
                order_type_t type = (order_type_t) zmq::get_uint8 (buff);
                buff += sizeof (uint8_t);
                price_t price = zmq::get_uint16 (buff);
                buff += sizeof (uint16_t);
                volume_t volume = zmq::get_uint16 (buff);
                callback_->order (order_id, type, price, volume);
                break;
            }
        case msg_type_order_confirmation:
            {
                assert (size == 4);
                order_id_t order_id = zmq::get_uint32 (buff);
                callback_->order_confirmation (order_id);
                break;
            }
        case msg_type_trade:
            {
                assert (size == 8);
                order_id_t order_id = zmq::get_uint32 (buff);
                buff += sizeof (uint32_t);
                price_t price = zmq::get_uint16 (buff);
                buff += sizeof (uint16_t);
                volume_t volume = zmq::get_uint16 (buff);
                callback_->trade (order_id, price, volume);
                break;
            }
        case msg_type_quote:
            {
                assert (size == 4);
                price_t bid = zmq::get_uint16 (buff);
                buff += sizeof (uint16_t);
                price_t ask = zmq::get_uint16 (buff);
                callback_->quote (bid, ask);
                break;
            }
        case msg_type_throughput:
            {
                assert (size == 9);
                uint8_t meter_id = zmq::get_uint8 (buff);
                buff += sizeof (uint8_t);
                uint64_t throughput = zmq::get_uint64 (buff);
                callback_->throughput (meter_id, throughput);
                break;
            }
        case msg_type_timestamp:
            {
                assert (size == 17);
                uint8_t meter_id = zmq::get_uint8 (buff);
                buff += sizeof (uint8_t);
                uint64_t correlation_id = zmq::get_uint64 (buff);
                buff += sizeof (uint64_t);
                uint64_t timestamp = zmq::get_uint64 (buff);
                callback_->timestamp (meter_id, correlation_id, timestamp);
                break;
            }
        default:
            assert (false);
        }
    }

}

#endif
