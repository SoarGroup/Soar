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
 * $Id: reb_params.h 1169 2002-12-10 05:34:27Z jazzfunk $
 * 
 * reb_params.h
 *
 * Parameters for the UMASS Ubots. 
*/

#ifndef _REB_PARAMS_H
#define _REB_PARAMS_H

#include <player.h>

void initialize_reb_params(void);

#define PLAYER_NUM_UBOT_ROBOT_TYPES 2


// here are some parameters for the ubot
typedef struct {
  char *Class;
  char *Subclass;
  int32_t MaxVelocity; // max translational velocity in mm/s
  int32_t MaxRVelocity; // max rotational velocity in deg/s
  int32_t RobotRadius; // radius in mm
  int32_t RobotAxleLength; // length from wheel contact to contact in mm
  double PulsesPerMM; // number of pulses per MM
  int32_t PulsesPerMMF; // fixed point of previous (previous * REB_FIXED_FACTOR)
  double MMPerPulses; // for completeness, inverse of previous, reduce # of floating ops
  int32_t MMPerPulsesF;
  double PulsesPerMMMS; // previous constants adjusted for time factor in velocity
  int32_t PulsesPerMMMSF;
  double MMPerPulsesMS;
  int32_t MMPerPulsesMSF;
  uint16_t NumberIRSensors;
  int16_t ir_pose[PLAYER_IR_MAX_SAMPLES][3]; // each row is {x, y, theta} in robo-centric coords (mm, mm, deg)
  uint16_t pos_kp, pos_ki, pos_kd;
} UBotRobotParams_t;
  
extern UBotRobotParams_t PlayerUBotRobotParams[];

#endif
