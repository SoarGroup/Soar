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
 * Copyright (C) 2002
 * John Sweeney, Laboratory for Perceptual Robotics, UMASS, Amherst
 *
 * $Id: reb_params.cc 1169 2002-12-10 05:34:27Z jazzfunk $
 * 
 * reb_params.cc
 *
 * Parameters for the UMass UBots
*/
#include <reb_params.h>

UBotRobotParams_t ubot_slow_params = 
{
  "UBot",  //Class
  "slow", //Subclass
  700, //Max Trans Velocity
  150, // MAX Rot Velocity
  90, // radius in mm
  139, // axle length in mm
  // the following is # of pulese per mm
  // derived as (pulses in one rev)/(wheel diameter*pi)
  // for the slow ubot the gear reduction is 4*43*16*2:1 = 5504:1
  // the wheel diam is 72 mm
  24.333022, // pulses per mm
  //  24333022, // previous * (REB_FIXED_FACTOR)
  243330,
  0.041096416, // inverse of previous= mm per pulse
  //  41096,
  411,
  0.24333022, // this is pulses/mm * 0.01 s -> pulses/mm 10 ms
  //  243330,
  2433,
  0.0004109641, // inverse of previous
  //  411,
  4,
  8, // 8 IRs
  {
    {35, 0, 0},
    {25, 25, 45},
    {0, 35, 90},
    {-25, 25, 135},
    {-35, 0, 180},
    {-25, -25, 225},
    {0, -35, 270},
    {25, -25, 315},
  },
  100,0,10 // position mode kp,ki,kd
};

UBotRobotParams_t ubot_fast_params = 
{
  "UBot",  //Class
  "fast", //Subclass
  700, //Max Trans Velocity
  150, // MAX Rot Velocity
  90, // radius in mm
  139, // axle length in mm
  // the following is # of pulese per mm
  // derived as (pulses in one rev)/(wheel diameter*pi)
  // for fast ubot: 4 (quad encoding) * 14 (23/1 series has 14:1 reduction) *
  // 16 (HE encoder has 16 pulse/rev) * 2 (reduction from gear to wheel) =
  // 4*14*16*2 = 1792 : 1
  // the wheel diam is 72 mm
  7.9223794,
  //  7922379,
  79224,
  0.1262247,
  //  126225,
  1262,
  0.079223794,
  //  79224,
  792,
  0.001262247,
  //  1262,
  13,
  8, // number of IRs
  {
    {35, 0, 0},
    {25, 25, 45},
    {0, 35, 90},
    {-25, 25, 135},
    {-35, 0, 180},
    {-25, -25, 225},
    {0, -35, 270},
    {25, -25, 315},
  },
  1200,0,10 //positoin mode kp, ki, kd
};



UBotRobotParams_t PlayerUBotRobotParams[PLAYER_NUM_UBOT_ROBOT_TYPES];

void
initialize_reb_params(void)
{
  PlayerUBotRobotParams[0] = ubot_slow_params;
  PlayerUBotRobotParams[1] = ubot_fast_params;
}
