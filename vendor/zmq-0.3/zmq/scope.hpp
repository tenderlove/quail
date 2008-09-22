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

#ifndef __ZMQ_SCOPE_HPP_INCLUDED__
#define __ZMQ_SCOPE_HPP_INCLUDED__

namespace zmq
{

    enum scope_t
    {
        //  Local scope means that the object is visible only within
        //  the engine that created it.
        scope_local,

        //  Process scope means that the object is visible to all the engines
        //  within the process registered with the same dispatcher object.
        scope_process,

        //  Global scope means that the object is visible to all the 0MQ
        //  processes registered with the same global_locator.
        scope_global
    };

}

#endif
