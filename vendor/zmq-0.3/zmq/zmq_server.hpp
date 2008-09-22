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

#ifndef __ZMQ_ZMQ_SERVER_HPP_INCLUDED__
#define __ZMQ_ZMQ_SERVER_HPP_INCLUDED__

namespace zmq
{

    //  Enumerates individual commands passed between process locator and
    //  global locator. Maps the symbolic names to command IDs.

    enum
    {
        create_exchange_id = 1,
        create_exchange_ok_id = 2,
        create_queue_id = 3,
        create_queue_ok_id = 4,
        get_exchange_id = 5,
        get_exchange_ok_id = 6,
        get_queue_id = 7,
        get_queue_ok_id = 8,
        fail_id = 9
    };

}

#endif
