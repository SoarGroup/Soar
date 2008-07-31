/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000  
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

/*
 * $Id: sip.h 4308 2007-12-13 20:23:08Z gerkey $
 *
 * part of the P2OS parser.  methods for filling and parsing server
 * information packets (SIPs)
 */
#ifndef _SIP_H
#define _SIP_H

#include <limits.h>

#include "erratic.h"


class erSIP 
{
 private:
  int PositionChange( unsigned short, unsigned short );
  int param_idx; // index of our robot's data in the parameter table

 public:
  // these values are returned in every standard SIP
  bool lwstall, rwstall;
  unsigned char status, battery;
  unsigned short ptu, timer, rawxpos; 
  unsigned short rawypos;
  short angle, lvel, rvel, control;
  int xpos, ypos;
  int x_offset,y_offset,angle_offset;

  bool Parse( unsigned char *buffer, int length );
  void Print();
  void Fill(player_erratic_data_t* data);

  erSIP(int idx) 
  {
    param_idx = idx;

    xpos = INT_MAX;
    ypos = INT_MAX;
  }
};

#endif
