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

#ifndef __PERF_TIME_HPP_INCLUDED__
#define __PERF_TIME_HPP_INCLUDED__

#include <assert.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>

#include "../../zmq/stdint.hpp"

namespace perf
{

    //  Time instance in nanoseconds.
    //  For measurement purposes the exact point when the timer started
    //  (e.g. midnight January 1, 1970) is irrelevant. The only requirement
    //  is that all times are measured from the same starting point.
    typedef uint64_t time_instant_t;

#if (defined (__GNUC__) && (defined (__i386__) || defined (__x86_64__)))
    //  Retrieves current time in processor ticks. This function is intended
    //  for internal usage - use 'now' function instead.
    inline uint64_t now_ticks ()
    {
        uint32_t low;
        uint32_t high;
        __asm__ volatile ("rdtsc"
            : "=a" (low), "=d" (high));
        return (uint64_t) high << 32 | low;
    }
#endif

    //  Retrieves current time in nanoseconds. This function is intended
    //  for internal usage - use 'now' function instead.
    inline uint64_t now_nsecs ()
    {
        struct timeval tv;
        int rc = gettimeofday (&tv, NULL);
        assert (rc == 0);
        return tv.tv_sec * (uint64_t) 1000000000 + tv.tv_usec * 1000;
    }

#if (defined (__GNUC__) && (defined (__i386__) || defined (__x86_64__)))
    //  Precomputes CPU frequency (in Hz). If ferquency measured in busy loop
    //  is too far (more than +- 10%) away from freq measured in sleep 
    //  function returns 0.
    inline uint64_t estimate_cpu_frequency ()
    {
        uint64_t cpu_frequency = 0;

        //  Measure frequency with busy loop.
        uint64_t start_nsecs = now_nsecs ();
        uint64_t start_ticks = now_ticks ();
        for (volatile int i = 0; i != 1000000000; i ++);
        uint64_t end_nsecs = now_nsecs ();
        uint64_t end_ticks = now_ticks ();

        uint64_t ticks = end_ticks - start_ticks;
        uint64_t nsecs = end_nsecs - start_nsecs;
        uint64_t busy_frq = ticks * 1000000000 / nsecs;

        std::cout << "CPU frequency measured with busy loop: " << busy_frq 
            << " [Hz]" << std::endl;
        
        //  Measure frequency with sleep.
        start_nsecs = now_nsecs ();
        start_ticks = now_ticks ();
        usleep (4000000);
        end_nsecs = now_nsecs ();
        end_ticks = now_ticks ();
        ticks = end_ticks - start_ticks;
        nsecs = end_nsecs - start_nsecs;
        uint64_t sleep_frq = ticks * 1000000000 / nsecs;

        std::cout << "CPU frequency measured with sleep: " << sleep_frq 
            << " [Hz]" << std::endl << std::endl;

        //  If the two frequencies are too far apart, there's a problem
        //  somewhere - presumably CPU frequency is being lowered
        //  as a power saving measure. However, this test doesn't seem
        //  to be sufficient. It may succeed even if power saving
        //  measures are on.
        if (busy_frq > sleep_frq * 1.1 && sleep_frq < busy_frq * 1.1) {
            std::cerr << "Difference more than +-10%!" << std::endl;
            std::cerr << "It looks like while your CPU is in idle, frequency "
                << "is being lowered. Probably \nby some power saving "
                << "enhancement. Please turn off all cpu frequency "
                << "lowering\nleverages and rerun the program." << std::endl;
            return 0;
        }

        // Return average from busy and sleep frequency.
        cpu_frequency = (busy_frq + sleep_frq) / 2;

        return cpu_frequency;
    }
#endif

    //  Get current time in nanosecond resolution.
    inline time_instant_t now ()
    {
#if (defined (__GNUC__) && defined (PERF_CPU_FREQUENCY) && \
    (defined (__i386__) || defined (__x86_64__)))

        //  When function is called for the first time, set timestamps to zero
        //  so that they'll be recomputed below.
        static uint64_t last_nsecs = 0;
        static uint64_t last_ticks = 0;

        //  Get current time (in CPU ticks).
        uint64_t current_ticks = now_ticks ();

        //  Find out whether one second has already elapsed since the last
        //  system time measurement. If so, measure the system time anew.
        if (current_ticks - last_ticks >= PERF_CPU_FREQUENCY) {
            last_nsecs = now_nsecs ();
            last_ticks = now_ticks ();
            current_ticks = last_ticks;
        }

        //  Return the sum of last measured system time and the ticks
        //  elapsed since then.
        return last_nsecs + ((current_ticks - last_ticks) * 1000000000 /
            PERF_CPU_FREQUENCY);
#else
        return now_nsecs ();
#endif
    }
}

#endif
