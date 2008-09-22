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

#include <iostream>

#include "./time.hpp"

using namespace std;

int main (void)
{

    //  Estimate CPU freq with perf function.
    uint64_t cpu_freq = perf::estimate_cpu_frequency ();

    //  If 0 is returned it means that CPU frequency between busy and idle CPU 
    //  states is changed singnificantly. Therefore can not be measured.
    if (cpu_freq == 0) {
        return 1;

    }

    //  If PERF_CPU_FREQUENCY macro is defined while compiling perf on 
    //  i386 and x86_64 platforms RTDSC will be used for time measuring.
    cout << "Please export CXXFLAGS=-DPERF_CPU_FREQUENCY=" << cpu_freq 
        << " and run\nconfigure and make in the top of the source tree again."
        << endl;

    return 0;
}
