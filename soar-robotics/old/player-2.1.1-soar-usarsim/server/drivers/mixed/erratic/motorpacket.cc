// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:1; -*-

/**
	*  Videre Erratic robot driver for Player
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
**/

#include <stdio.h>
#include <limits.h>
#include <math.h>  /* rint(3) */
#include <sys/types.h>
#include <netinet/in.h>
#include "motorpacket.h"
#include <stdlib.h> /* for abs() */
#include <unistd.h>

int ErraticMotorPacket::PositionChange( unsigned short from, unsigned short to ) 
{
  int diff1, diff2;

  /* find difference in two directions and pick shortest */
  if ( to > from ) {
    diff1 = to - from;
    diff2 = - ( from + 4096 - to );
  }
  else {
    diff1 = to - from;
    diff2 = 4096 - from + to;
  }

  if ( abs(diff1) < abs(diff2) ) 
    return(diff1);
  else
    return(diff2);

}

void ErraticMotorPacket::Print() 
{
  printf("lwstall:%d rwstall:%d\n", lwstall, rwstall);

  printf("status: 0x%x", status);
  printf("battery: %d\n", battery);
  printf("xpos: %d ypos:%d ptu:%hu timer:%hu\n", xpos, ypos, ptu, timer);
  printf("angle: %d lvel: %d rvel: %d control: %d\n", angle, lvel, rvel, control);
}

// Parses and absorbs a standard packet from the robot
bool ErraticMotorPacket::Parse( unsigned char *buffer, int length ) 
{
  int cnt = 0, change;
  unsigned short newxpos, newypos;

  // Check type and length
  if (length < 20) return false;

  status = buffer[cnt];
  cnt += sizeof(unsigned char);

  // This driver does its own integration, and only cares for the lower bits 
  // of the odometry updates
  // Integers from robot are little-endian
  newxpos = buffer[cnt] + 0x100*(buffer[cnt+1]&0x0F);

  if (xpos!=INT_MAX) {
    change = (int) rint(PositionChange( rawxpos, newxpos ) * RobotParams[param_idx]->DistConvFactor);
    //printf("xchange: %i ", change);
    if (abs(change)>100)
      ;//PLAYER_WARN1("invalid odometry change [%d]; odometry values are tainted", change);
    else
      xpos += change;
  }
  else {
    //printf("xpos was INT_MAX\n");
		xpos = 0;
	}
  rawxpos = newxpos;
  cnt += 3;

  
  newypos = buffer[cnt] + 0x100*(buffer[cnt+1]&0x0F);

  if (ypos!=INT_MAX) {
    change = (int) rint(PositionChange( rawypos, newypos ) * RobotParams[param_idx]->DistConvFactor);
    if (abs(change)>100)
      ;//PLAYER_WARN1("invalid odometry change [%d]; odometry values are tainted", change);
    else
      ypos += change;
  }
  else
    ypos = 0;
  rawypos = newypos;
  cnt += 3;

	//if (debug_mode)
  //  printf("Just parsed, new xpos: %i ypos: %i\n", xpos, ypos);

  //  angle = (short)
  //    rint(((short)(buffer[cnt] | (buffer[cnt+1] << 8))) *
  //	 M_PI/2048.0 * 180.0/M_PI);
  angle = (short)(buffer[cnt] | (buffer[cnt+1] << 8)); // keep as 4096 / full turn

  cnt += sizeof(short);

  lvel = (short)
    rint(((short)(buffer[cnt] | (buffer[cnt+1] << 8))) *
	 RobotParams[param_idx]->VelConvFactor);
  cnt += sizeof(short);

  rvel = (short)
    rint(((short)(buffer[cnt] | (buffer[cnt+1] << 8))) *
	 RobotParams[param_idx]->VelConvFactor);
  cnt += sizeof(short);

  battery = buffer[cnt];
  cnt += sizeof(unsigned char);
  
  lwstall = buffer[cnt] & 0x01;
  cnt += sizeof(unsigned char);
  
  rwstall = buffer[cnt] & 0x01;
  cnt += sizeof(unsigned char);

  control = (short)
    rint(((short)(buffer[cnt] | (buffer[cnt+1] << 8))) *
	 RobotParams[param_idx]->AngleConvFactor);
  cnt += sizeof(short);

	return true;
}

// Spits out information that was previously parsed
void ErraticMotorPacket::Fill(player_erratic_data_t* data)
{
  // Odometry data
  {
    // initialize position to current offset
    data->position.pos.px = (double)(this->xpos - this->x_offset) / 1e3;
    data->position.pos.py = (double)(this->ypos - this->y_offset) / 1e3;
    // now transform current position by rotation if there is one
    // and add to offset
    if(this->angle_offset != 0) 
      {
	double rot = ATOR(this->angle_offset);    // convert rotation to radians
	double ax = data->position.pos.px;
	double ay = data->position.pos.py;
	data->position.pos.px =  ax * cos(rot) + ay * sin(rot);
	data->position.pos.py = -ax * sin(rot) + ay * cos(rot);
	data->position.pos.pa = ATOR(angle - this->angle_offset);
      }
    else
      data->position.pos.pa = ATOR(this->angle);

    data->position.vel.px = (((this->lvel) + (this->rvel) ) / 2) / 1e3;
    data->position.vel.py = 0.0;
    data->position.vel.pa = (0.596*(double)(this->rvel - this->lvel) /
			     (2.0/RobotParams[param_idx]->DiffConvFactor));
    data->position.stall = (unsigned char)(this->lwstall || this->rwstall);
  }

  // Battery data
  {
    data->power.valid = PLAYER_POWER_MASK_VOLTS | PLAYER_POWER_MASK_PERCENT;
    data->power.volts = this->battery / 1e1;
    data->power.percent = 1e2 * (data->power.volts / VIDERE_NOMINAL_VOLTAGE);
  }
}

