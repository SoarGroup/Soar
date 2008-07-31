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

/***************************************************************************
 * Desc: Player v2.0 C++ client
 * Authors: Brad Kratochvil, Toby Collett
 *
 * Date: 23 Sep 2005
 # CVS: $Id: utility.h 4100 2007-07-10 09:01:53Z thjc $
 **************************************************************************/


#ifndef PLAYERCC_UTILITY_H
#define PLAYERCC_UTILITY_H
namespace PlayerCc
{
/** @ingroup player_clientlib_cplusplus
    @addtogroup player_clientlib_utility Constants and utility functions
    @brief Helper functions when using the library.
 @{

 */

/// The default port number for PlayerClient
const int PLAYER_PORTNUM = 6665;
/// The default hostname for PlayerClient
const std::string PLAYER_HOSTNAME = "localhost";

// Since they are inline, these functions are as efficient as DEFINES,
// but now they have the benefit of type checking!

/// Convert radians to degrees
inline double rtod(double r)
{
  return r * 180.0 / M_PI;
}

/// Convert degrees to radians
inline double dtor(double r)
{
  return r * M_PI / 180.0;
}

/// Normalize angle to domain -pi, pi
inline double normalize(double z)
{
  return atan2(sin(z), cos(z));
}

/// Return the minimum of a, b
template<typename T>
inline T min(T a, T b)
{
  if (a < b)
    return a;
  else
    return b;
}

/// Return the maximum of a, b
template<typename T>
inline T max(T a, T b)
{
  if (a > b)
    return a;
  else
    return b;
}

/// Limit a value to the range of min, max
template<typename T>
inline T limit(T a, T min, T max)
{
  if (a < min)
    return min;
  else if (a > max)
    return max;
  else
    return a;
}

/** }@ (utility) */

} // namespace

#endif

