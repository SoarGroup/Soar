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
 * $Id: position1dproxy.cc 4227 2007-10-24 22:32:04Z thjc $
 */

#include "playerc++.h"

using namespace PlayerCc;

Position1dProxy::Position1dProxy(PlayerClient *aPc, uint32_t aIndex)
  : ClientProxy(aPc, aIndex),
  mDevice(NULL)
{
  Subscribe(aIndex);
  // how can I get this into the clientproxy.cc?
  // right now, we're dependent on knowing its device type
  mInfo = &(mDevice->info);
}

Position1dProxy::~Position1dProxy()
{
  Unsubscribe();
}

void
Position1dProxy::Subscribe(uint32_t aIndex)
{
  scoped_lock_t lock(mPc->mMutex);
  mDevice = playerc_position1d_create(mClient, aIndex);
  if (NULL==mDevice)
    throw PlayerError("Position1dProxy::Position1dProxy()", "could not create");

  if (0 != playerc_position1d_subscribe(mDevice, PLAYER_OPEN_MODE))
    throw PlayerError("Position1dProxy::Position1dProxy()", "could not subscribe");
}

void
Position1dProxy::Unsubscribe()
{
  assert(NULL!=mDevice);
  scoped_lock_t lock(mPc->mMutex);
  playerc_position1d_unsubscribe(mDevice);
  playerc_position1d_destroy(mDevice);
  mDevice = NULL;
}

std::ostream&
std::operator << (std::ostream &os, const PlayerCc::Position1dProxy &c)
{
  os << "#Position1D (" << c.GetInterface() << ":" << c.GetIndex() << ")" << std::endl;
  os << "#pos\tvel\tstall" << std::endl;
  os << c.GetPos() << " " << c.GetVel() << " " << c.GetStall() << std::endl;
  return os;
}

void
Position1dProxy::SetSpeed(double aVel)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_position1d_set_cmd_vel(mDevice,aVel,0);
}

void
Position1dProxy::GoTo(double aPos, double aVel=0)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_position1d_set_cmd_pos_with_vel(mDevice, aPos, aVel,0);
}

void
Position1dProxy::SetMotorEnable(bool aEnable)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_position1d_enable(mDevice,aEnable);
}

void
Position1dProxy::SetOdometry(double aPos)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_position1d_set_odom(mDevice, aPos);
}


void
Position1dProxy::RequestGeom()
{
  boost::mutex::scoped_lock lock(mPc->mMutex);
  if (0 != playerc_position1d_get_geom(mDevice))
    throw PlayerError("Position1dProxy::RequestGeom()", "error getting geom");
  return;
}
