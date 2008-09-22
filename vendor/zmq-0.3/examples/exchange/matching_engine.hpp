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

#ifndef __EXCHANGE_MATCHING_ENGINE_HPP_INCLUDED__
#define __EXCHANGE_MATCHING_ENGINE_HPP_INCLUDED__

#include <assert.h>
#include <stdio.h>
#include <vector>
#include <deque>

#include "common.hpp"

namespace exchange
{

    //  Matching engine for a particular product.
    //
    //  Given the assumption that the order prices are densly packed around
    //  the current market price, the complexity of matching is O(1) with
    //  respect to the number of orders in the orderbook.

    class matching_engine_t
    {
    private:

        //  Lower limit is the maximal out-of-range price
        //  Upper limit is the minimal out-of-range price
        //  If we later choose to adjust the price range dynamically,
        //  these should be standard member variables rather than constants.
        enum
        {
            lower_limit = 0,
            upper_limit = 1000
        };

    public:

        inline matching_engine_t () :
            min_ask (upper_limit),
            max_bid (lower_limit),
            orderbook (upper_limit - lower_limit - 1)
        {
        }

        template <typename T> bool ask (T *callback_, order_id_t order_id_,
            price_t price_, volume_t volume_)
        {
            assert (price_ > lower_limit && price_ < upper_limit);
            assert (volume_ > 0);

            //  Remember the original max_bid & min_ask so that we can
            //  generate the quote if they are changed
            price_t old_max_bid = max_bid;
            price_t old_min_ask = min_ask;

            //  If at least one trade was generated, this variable will be true
            bool trades_sent = false;

            while (true) {

                //  If there's no matching bid, store the order and return
                if (price_ > max_bid) {
                    entry_t entry = {volume_, order_id_};
                    orderbook [price_ - lower_limit - 1].push_back (entry);
                    min_ask = std::min (min_ask, price_);
                    break;
                }

                //  Get the bids with maximal prices
                entries_t &entries = orderbook [max_bid - lower_limit - 1];

                //  Amount traded at this price level
                volume_t traded = 0;

                while (!entries.empty ()) {

                    //  Get the oldest available bid
                    entry_t &entry = entries.front ();

                    //  Determine trade volume
                    volume_t trade_volume = std::min (volume_, entry.volume);

                    //  Execute the trade on bid
                    callback_->traded (entry.order_id, max_bid, trade_volume);
                    traded += trade_volume;
		    trades_sent = true;

                    //  Adjust the bid: If it's fully executed, delete it
                    entry.volume -= trade_volume;
                    if (entry.volume == 0)
                        entries.pop_front ();

                    //  Adjust the ask: If it's fully executed, exit
                    volume_ -= trade_volume;
                    if (volume_ == 0) 
                        break;
                }

                //  Execute the trade on ask
                if (traded) {
                    callback_->traded (order_id_, max_bid, traded);
                    trades_sent = true;
                }

                //  If order is fully executed, exit the loop
                if (volume_ == 0)
                    break;

                //  We have executed all the bids with best price at this point
                //  Move to next best bid price level
                max_bid --;
            }

            //  Now we find the max_bid. The algorithm seems unefficient as it
            //  performs linear search - O(n) - however:
            //
            //  1. The prices tend to be packed around the market price,
            //     consequently n is most probably 0 or 1 (thus we are in fact
            //     getting constant-time algorithm )
            //  2. We would have to find actual max_bid anyway after the arrival
            //     of next ask order, so the search is inevitable.
            while (max_bid != lower_limit &&
                  orderbook [max_bid - lower_limit - 1].empty ())
                max_bid --;

            //  Generate the quote if needed
            if (max_bid != old_max_bid || min_ask != old_min_ask)
                callback_->quoted (max_bid, min_ask);

            return trades_sent;
        }

        template <typename T> bool bid (T *callback_, order_id_t order_id_,
            price_t price_, volume_t volume_)
        {
            assert (price_ > lower_limit && price_ < upper_limit);
            assert (volume_ > 0);

            //  Remember the original max_bid & min_ask so that we can
            //  generate the quote if they are changed
            price_t old_max_bid = max_bid;
            price_t old_min_ask = min_ask;

            //  If at least one trade was executed, this variable will be true
            bool trades_sent = false;

            while (true) {

                //  If there's no matching ask, store the order and return
                if (price_ < min_ask) {
                    entry_t entry = {volume_, order_id_};
                    orderbook [price_ - lower_limit - 1].push_back (entry);
                    max_bid = std::max (max_bid, price_);
                    break;
                }

                //  Get the asks with minimal prices
                entries_t &entries = orderbook [min_ask - lower_limit - 1];

                //  Amount traded at this price level
                volume_t traded = 0;

                while (!entries.empty ()) {

                    //  Get the oldest available ask
                    entry_t &entry = entries.front ();

                    //  Determine trade volume
                    volume_t trade_volume = std::min (volume_, entry.volume);

                    //  Execute the trade on ask
                    callback_->traded (entry.order_id, min_ask, trade_volume);
                    traded += trade_volume;
                    trades_sent = true;

                    //  Adjust the ask: If it's fully executed, delete it
                    entry.volume -= trade_volume;
                    if (entry.volume == 0)
                        entries.pop_front ();

                    //  Adjust the bid: If it's fully executed, exit
                    volume_ -= trade_volume;
                    if (volume_ == 0)
                        break;
                }

                //  Execute the trade on bid
                if (traded) {
                    callback_->traded (order_id_, min_ask, traded);
                    trades_sent = true;
                }

                //  If order is fully executed, exit the loop
                if (volume_ == 0)
                    break;

                //  We have executed all the asks with best price at this point
                //  Move to next best ask price level
                min_ask ++;
            }

            //  Now we find the min_ask. The algorithm seems unefficient as it
            //  performs linear search - O(n) - however:
            //
            //  1. The prices tend to be packed around the market price,
            //     consequently n is most probably 0 or 1 (thus we are in fact
            //     getting constant-time algorithm )
            //  2. We would have to find actual min_ask anyway after the arrival
            //     of next bid order, so the search is inevitable.
            while (min_ask != upper_limit &&
                  orderbook [min_ask - lower_limit - 1].empty ())
                min_ask ++;

            //  Generate the quote if needed
            if (max_bid != old_max_bid || min_ask != old_min_ask)
                callback_->quoted (max_bid, min_ask);

            return trades_sent;
        }
    private:

        //  Represents the state of particular order in the order book.
        //  Volume is the remaining volume to trade rather than
        //  full order volume.
        struct entry_t
        {
            volume_t volume;
            order_id_t order_id;
        };

        //  Set of all orders with the same price
        typedef std::deque <entry_t> entries_t;

        //  Set of all orders
        typedef std::vector <entries_t> orderbook_t;

        //  Minimal available ask price & maximal available bid price
        //  If min_ask == upper_limit, there are no asks in the orderbook
        //  If max_bid == lower_limit, there are no bids in the orderbook
        price_t min_ask;
        price_t max_bid;

        //  The first element in the orderbook corresponds to the minimal
        //  possible price, second one to the second minimal price etc.
        orderbook_t orderbook;
    };

}

#endif
