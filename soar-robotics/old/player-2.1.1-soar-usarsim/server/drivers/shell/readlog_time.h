/*
 *  Player - One Hell of a Robot Server
 *     Brian Gerkey, Kasper Stoy, Richard Vaughan, & Andrew Howard
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
///////////////////////////////////////////////////////////////////////////
//
// Desc: Log file time
// Author: Andrew Howard
// Date: 28 May 2003
// CVS: $Id: readlog_time.h 3333 2006-01-20 21:26:31Z gerkey $
//
///////////////////////////////////////////////////////////////////////////

#ifndef READLOG_TIME_H
#define READLOG_TIME_H

#include <libplayercore/playertime.h>

// Incremental navigation driver
class ReadLogTime : public PlayerTime
{
  // Constructor
  public: ReadLogTime();

  // Destructor
  public: virtual ~ReadLogTime();

  // Get the current time
  public: int GetTime(struct timeval* time);

  // Get the current time
  public: int GetTimeDouble(double* time);
};


// Some readlog related global vars
extern struct timeval ReadLogTime_time;
extern double ReadLogTime_timeDouble;

#endif
