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
 * $Id: position3dproxy.cc 4227 2007-10-24 22:32:04Z thjc $
 */

#include "playerc++.h"

using namespace PlayerCc;

Position3dProxy::Position3dProxy(PlayerClient *aPc, uint32_t aIndex)
  : ClientProxy(aPc, aIndex),
  mDevice(NULL)
{
  Subscribe(aIndex);
  // how can I get this into the clientproxy.cc?
  // right now, we're dependent on knowing its device type
  mInfo = &(mDevice->info);
}

Position3dProxy::~Position3dProxy()
{
  Unsubscribe();
}

void
Position3dProxy::Subscribe(uint32_t aIndex)
{
  scoped_lock_t lock(mPc->mMutex);
  mDevice = playerc_position3d_create(mClient, aIndex);
  if (NULL==mDevice)
    throw PlayerError("Position3dProxy::Position3dProxy()", "could not create");

  if (0 != playerc_position3d_subscribe(mDevice, PLAYER_OPEN_MODE))
    throw PlayerError("Position3dProxy::Position3dProxy()", "could not subscribe");
}

void
Position3dProxy::Unsubscribe()
{
  assert(NULL!=mDevice);
  scoped_lock_t lock(mPc->mMutex);
  playerc_position3d_unsubscribe(mDevice);
  playerc_position3d_destroy(mDevice);
  mDevice = NULL;
}

std::ostream&
std::operator << (std::ostream &os, const PlayerCc::Position3dProxy &c)
{
  os << "#Position3d (" << c.GetInterface() << ":" << c.GetIndex() << ")" << std::endl;
  os << "#X\tY\tZ\tRoll\tPitch\tYaw" << std::endl;
  os << "Pos: " << c.GetXPos() << " " << c.GetYPos() << " " << c.GetZPos() << " "
     << c.GetRoll() << " " << c.GetPitch() << " " << c.GetYaw() << std::endl;
  os << "Vel: " << c.GetXSpeed() << " " << c.GetYSpeed() << " " << c.GetZSpeed()
     << " " << c.GetRollSpeed() << " " << c.GetPitchSpeed() << " " << c.GetYawSpeed()
     << std::endl;
  return os;
}

void
Position3dProxy::SetSpeed(double aXSpeed, double aYSpeed, double aZSpeed,
                          double aRollSpeed, double aPitchSpeed, double aYawSpeed)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_position3d_set_velocity(mDevice, aXSpeed, aYSpeed, aZSpeed,
                                  aRollSpeed, aPitchSpeed, aYawSpeed, 0);
}

void
Position3dProxy::GoTo(player_pose3d_t aPos, player_pose3d_t aVel)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_position3d_set_pose_with_vel(mDevice, aPos, aVel);
}

void
Position3dProxy::SetMotorEnable(bool aEnable)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_position3d_enable(mDevice,aEnable);
}

void
Position3dProxy::SelectVelocityControl(int mode)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_position3d_set_vel_mode(mDevice, mode);
}

void Position3dProxy::SetOdometry(double aX, double aY, double aZ,
                     double aRoll, double aPitch, double aYaw)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_position3d_set_odom(mDevice, aX, aY, aZ, aRoll, aPitch, aYaw);
}

void Position3dProxy::ResetOdometry()
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_position3d_reset_odom(mDevice);
}


void
Position3dProxy::RequestGeom()
{
  boost::mutex::scoped_lock lock(mPc->mMutex);

  if (0 != playerc_position3d_get_geom(mDevice))
    throw PlayerError("Position3dProxy::RequestGeom()", "error getting geom");
  return;
}
