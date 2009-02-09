/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000-2003
 *     Brian Gerkey, Kasper Stoy, Richard Vaughan, & Andrew Howard
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/********************************************************************
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ********************************************************************/

/*
 * $Id: playerc++.cc 4127 2007-08-20 19:42:49Z thjc $
 */

#include "playerc++.h"

/** @ingroup clientlibs
    @addtogroup player_clientlib_cplusplus libplayerc++
    @brief A C++ client library for the @ref util_player

The C++ library is built on a "service proxy" model in which the client
maintains local objects that are proxies for remote services.  This library
wraps the functionality of @ref player_clientlib_libplayerc with a more
friendly C++ API.

 *  The core of libplayerc++ is based around the following classes
 *  - PlayerCc::PlayerClient
 *  - PlayerCc::ClientProxy
 *  - PlayerCc::PlayerError

Be sure to see @ref cplusplus_example "this example".
 */

 /** @ingroup player_clientlib_cplusplus
    @addtogroup cplusplus_example libplayerc++ example
    @brief An example of using libplayerc++

The C++ library is built on a "service proxy" model in which the client
maintains local objects that are proxies for remote services.  There are
two kinds of proxies: the special server proxy  PlayerClient and the
various device-specific proxies.  Each kind of proxy is implemented as a
separate class.  The user first creates a PlayerClient proxy and uses
it to establish a connection to a Player server.  Next, the proxies of the
appropriate device-specific types are created and initialized using the
existing PlayerClient proxy.  To make this process concrete, consider
the following simple example (for clarity, we omit some error-checking):

@include example0.cc

Compile this program like so:
@verbatim
$ g++ -o example0 `pkg-config --cflags playerc++` example0.cc `pkg-config --libs playerc++`
@endverbatim

Be sure that libplayerc++ is installed somewhere that pkg-config can find it.

This program performs simple (and bad) sonar-based obstacle avoidance with
a mobile robot .  First, a PlayerClient
proxy is created, using the default constructor to connect to the
server listening at @p localhost:6665.  Next, a SonarProxy is
created to control the sonars and a PositionProxy to control the
robot's motors.  The constructors for these objects use the existing
 PlayerClient proxy to establish access to the 0th @ref interface_sonar
and @ref interface_position2d devices, respectively. Finally, we enter
a simple loop that reads the current sonar state and writes appropriate
commands to the motors.

@section Using automake

An Automake package config file is included(playerc++.pc).  To use this in
your automake project, simply add the following to your configure.in or
configure.ac:

@verbatim
# Player C++ Library
PKG_CHECK_MODULES(PLAYERCC, playerc++)
AC_SUBST(PLAYERCC_CFLAGS)
AC_SUBST(PLAYERCC_LIBS)
@endverbatim

Then, in your Makefile.am you can add:
@verbatim
AM_CPPFLAGS += $(PLAYERCC_CFLAGS)
programname_LDFLAGS = $(PLAYERCC_LIBS)
@endverbatim

*/

/** @ingroup player_clientlib_cplusplus
    @addtogroup player_clientlib_multi Signals & multithreading
    @brief Boost signal and thread support

Along with providing access to the basic C functions of @ref player_clientlib_libplayerc
in a C++ fashion, libplayerc++ also provides additional functionality along
the lines of signaling and multithreading.  The multithreaded ability of
libplayerc++ allieves the developer from having to worry about allotting time
to handle messaging.  It also allows for the PlayerClient to act as a
messaging loop for event driven programs.  The signaling and multithreading
ability of libplayerc++ is built from the <a href="http://www.boost.org">
Boost c++ libraries</a>.  This is relevant because we will be using boost
semantincs for connecting the signals to the client.  Much of this functionality
can best be illustrated through the use of an example:

@include example1.cc

*/

/** @ingroup player_clientlib_cplusplus
    @addtogroup player_clientlib_mclient MultiClient
    @brief How to easily connect to multiple servers

The C++ does not have a specific Client class for connecting to multiple
servers.  This is because the task can be easily accomplished with classes
already contained in the STL.  The following example shows how this can be
done.  After the initial setup and adding the clients to the list, the
for_each loop needs to be called each time a multi-read is performed.

@include example3.cc

*/

std::ostream&
std::operator << (std::ostream& os, const player_point_2d_t& c)
{
  os << "point: " << c.px << "," << c.py;
  return os;
}

std::ostream&
std::operator << (std::ostream& os, const player_pose2d_t& c)
{
  os << "pose: " << c.px << "," << c.py << "," << c.pa;
  return os;
}

std::ostream&
std::operator << (std::ostream& os, const player_pose3d_t& c)
{
  os << "pose3d: " << c.px << "," << c.py << "," << c.pz << " "
     << c.proll << "," << c.ppitch << "," << c.pyaw;
  return os;
}

std::ostream&
std::operator << (std::ostream& os, const player_bbox2d_t& c)
{
  os << "bbox: " << c.sw << "," << c.sl;
  return os;
}

std::ostream&
std::operator << (std::ostream& os, const player_bbox3d_t& c)
{
  os << "bbox: " << c.sw << "," << c.sl << "," << c.sh;
  return os;
}

std::ostream&
std::operator << (std::ostream& os, const player_segment_t& c)
{
  os << "segment: (" << c.x0 << "," << c.y0 << ") - ("
     << c.x1 << "," << c.y1 << ")";
  return os;
}

std::ostream&
std::operator << (std::ostream& os, const player_extent2d_t& c)
{
  os << "extent: (" << c.x0 << "," << c.y0 << ") - ("
     << c.x1 << "," << c.y1 << ")";
  return os;
}

std::ostream&
std::operator << (std::ostream& os, const playerc_device_info_t& c)
{
  os << c.drivername << "("
     << interf_to_str(c.addr.interf) << ":" << c.addr.index
     << ")";
  return os;
}
