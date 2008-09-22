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

#ifndef __EXCHANGE_FREQUENCY_METER_HPP_INCLUDED__
#define __EXCHANGE_FREQUENCY_METER_HPP_INCLUDED__

#include "../../perf/helpers/time.hpp"

namespace exchange
{

    //  Class to measure the frequency as a specific event. The time window used
    //  has variable size to avoid need for system timers and keep the overall
    //  impact of the measurement on the performance low. 

    class frequency_meter_t
    {
    public:

        //  Create frequency meter. Window is the number of events
        //  after which frequency calculation is done.
        inline frequency_meter_t (uint64_t window_, uint8_t meter_id_) :
            current (window_),
            window (window_),
            last_time (0),
            meter_id (meter_id_)
        {
        }

        //  Measured event occured. If actual mesurement is made, callback
        //  fuction is invoked on the supplied object
        template <typename T> inline void event (T *callback_)
        {
            //  Once we've reached the end of the window...
            if (current == window) {

                //  Get current time [ns]
                uint64_t now_time = perf::now ();

                //  If there've been previous measurement
                if (last_time) {

                    //  Calculate and report the frequency
                    uint64_t frequency = window * 1000000000 / 
                        (now_time - last_time);
                    callback_->frequency (meter_id, frequency);
                }

                //  Remember the current time and start filling the window anew
                last_time = now_time;
                current = 0;
            }

            //  Advance the window
            current ++;
        }

    private:

        //  Current position in the window
        uint64_t current;

        //  Window size
        uint64_t window;

        //  Time of when the window was started (in CPU ticks)
        uint64_t last_time;

        //  ID of the measuring device
        uint8_t meter_id;
    };

}

#endif
