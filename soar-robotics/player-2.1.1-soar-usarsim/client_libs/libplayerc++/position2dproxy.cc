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
 * $Id: position2dproxy.cc 4227 2007-10-24 22:32:04Z thjc $
 */

#include "playerc++.h"

using namespace PlayerCc;

Position2dProxy::Position2dProxy(PlayerClient *aPc, uint32_t aIndex)
  : ClientProxy(aPc, aIndex),
  mDevice(NULL)
{
  Subscribe(aIndex);
  // how can I get this into the clientproxy.cc?
  // right now, we're dependent on knowing its device type
  mInfo = &(mDevice->info);
}

Position2dProxy::~Position2dProxy()
{
  Unsubscribe();
}

void
Position2dProxy::Subscribe(uint32_t aIndex)
{
  scoped_lock_t lock(mPc->mMutex);
  mDevice = playerc_position2d_create(mClient, aIndex);
  if (NULL==mDevice)
    throw PlayerError("Position2dProxy::Position2dProxy()", "could not create");

  if (0 != playerc_position2d_subscribe(mDevice, PLAYER_OPEN_MODE))
    throw PlayerError("Position2dProxy::Position2dProxy()", "could not subscribe");
}

void
Position2dProxy::Unsubscribe()
{
  assert(NULL!=mDevice);
  scoped_lock_t lock(mPc->mMutex);
  playerc_position2d_unsubscribe(mDevice);
  playerc_position2d_destroy(mDevice);
  mDevice = NULL;
}

std::ostream&
std::operator << (std::ostream &os, const PlayerCc::Position2dProxy &c)
{
  os << "#Position2D (" << c.GetInterface() << ":" << c.GetIndex() << ")" << std::endl;
  os << "#xpos\typos\ttheta\tspeed\tsidespeed\tturn\tstall" << std::endl;
  os << c.GetXPos() << " " << c.GetYPos() << " " << c.GetYaw() << " " ;
  os << c.GetXSpeed() << " " << c.GetYSpeed() << " " << c.GetYawSpeed() << " ";
  os << c.GetStall() << std::endl;
  return os;
}

void
Position2dProxy::SetSpeed(double aXSpeed, double aYSpeed, double aYawSpeed)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_position2d_set_cmd_vel(mDevice,aXSpeed,aYSpeed,aYawSpeed,1);
}


void Position2dProxy::GoTo(player_pose2d_t pos, player_pose2d_t vel)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_position2d_set_cmd_pose_with_vel(mDevice,pos,vel,1);
}


void
Position2dProxy::SetCarlike(double aXSpeed, double aDriveAngle)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_position2d_set_cmd_car(mDevice,aXSpeed,aDriveAngle);
}

void
Position2dProxy::SetMotorEnable(bool aEnable)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_position2d_enable(mDevice,aEnable);
}

void
Position2dProxy::ResetOdometry()
{
  SetOdometry(0,0,0);
}

void
Position2dProxy::SetOdometry(double aX, double aY, double aYaw)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_position2d_set_odom(mDevice,aX,aY,aYaw);
}


void
Position2dProxy::RequestGeom()
{
  boost::mutex::scoped_lock lock(mPc->mMutex);

  if (0 != playerc_position2d_get_geom(mDevice))
    throw PlayerError("Position2dProxy::RequestGeom()", "error getting geom");
  return;
}
