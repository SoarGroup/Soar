/**
  *  Videre Erratic robot driver for Player
  *
  *  Copyright (C) 2006
  *     Videre Design
  *  Copyright (C) 2000
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
**/

#ifndef _ROBOT_PARAMS_H
#define _ROBOT_PARAMS_H

#include "libplayercore/player.h"

void initialize_robot_params(void);

#define PLAYER_NUM_ROBOT_TYPES 30


typedef struct
{
  double AngleConvFactor; //
  const char* Class;
  double DiffConvFactor; //
  double DistConvFactor; //
  int FrontBumpers; //
  double GyroScaler; //
  int HasMoveCommand; //
  int Holonomic; //
  int IRNum; //
  int IRUnit; //
  int LaserFlipped; //
  const char* LaserIgnore;
  const char* LaserPort;
  int LaserPossessed; //
  int LaserPowerControlled; //
  int LaserTh; //
  int LaserX; //
  int LaserY; //
  int MaxRVelocity; //
  int MaxVelocity; //
  int NewTableSensingIR; //
  int NumFrontBumpers; //
  int NumRearBumpers; //
  double RangeConvFactor; //
  int RearBumpers; //
  int RequestEncoderPackets; //
  int RequestIOPackets; //
  int RobotDiagonal; //
  int RobotLength; //
  int RobotRadius; //
  int RobotWidth; //
  int RobotAxleOffset; //
  int RotAccel; //
  int RotDecel; //
  int RotVelMax; //
  int SettableAccsDecs; //
  int SettableVelMaxes; //
  const char* Subclass;
  int SwitchToBaudRate; //
  int TableSensingIR; //
  int TransAccel; //
  int TransDecel; //
  int TransVelMax; //
  int Vel2Divisor; //
  double VelConvFactor; //
  int NumSonars;
  player_pose3d_t sonar_pose[32];
  int NumIR;
  player_pose3d_t IRPose[8];
} RobotParams_t;


//extern RobotParams_t PlayerRobotParams[];
//extern RobotParams_t erratic_params;

extern RobotParams_t *RobotParams[];

#endif
