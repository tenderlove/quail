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

#ifndef __EXCHANGE_TICKER_HPP_INCLUDED__
#define __EXCHANGE_TICKER_HPP_INCLUDED__

#include <assert.h>

#include "./time.hpp"

namespace perf
{

    //  Class to produce ticks at specified frequency.

    class ticker_t
    {
    public:

        //  Initialise ticker. required frequency of ticks (in Hz)
        //  should be supplied.
        ticker_t (uint64_t tick_frequency_) : first_tick (true), next (0)
        {
            period_ns = 1000000000 / tick_frequency_;
        }

        //  Waits for the next tick. The ticks are "queued", meaning that
        //  if you miss the expected time, next call to the function will
        //  return immediately.
        void wait_for_tick ()
        {

            if (first_tick) {
                next = now () + period_ns;
                first_tick = false;
                return;
            }

            while (true) {
                uint64_t now_ns = now ();

                assert (now_ns <= next + 1000 * period_ns);

                if (now_ns >= next ) {
                    next += period_ns;
                    break;
                }
            }
        }

    public:

        //  If true, we haven't measured the time yet.
        bool first_tick;

        //  Next time (in ns) to issue an event on.
        uint64_t next;

        //  Interval between events in ns.
        uint64_t period_ns;
    };

}

#endif
