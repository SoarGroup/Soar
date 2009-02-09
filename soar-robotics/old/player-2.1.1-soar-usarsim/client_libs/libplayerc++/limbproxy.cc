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
 * $Id: limbproxy.cc 4227 2007-10-24 22:32:04Z thjc $
 *
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <cassert>
#include <sstream>
#include <iomanip>

#include "playerc++.h"
#include "debug.h"

using namespace PlayerCc;

LimbProxy::LimbProxy(PlayerClient *aPc, uint32_t aIndex)
  : ClientProxy(aPc, aIndex),
  mDevice(NULL)
{
  Subscribe(aIndex);
  mInfo = &(mDevice->info);
}

LimbProxy::~LimbProxy()
{
  Unsubscribe();
}

void LimbProxy::Subscribe(uint32_t aIndex)
{
  scoped_lock_t lock(mPc->mMutex);
  mDevice = playerc_limb_create(mClient, aIndex);
  if (mDevice==NULL)
    throw PlayerError("LimbProxy::LimbProxy()", "could not create");

  if (playerc_limb_subscribe(mDevice, PLAYER_OPEN_MODE) != 0)
    throw PlayerError("LimbProxy::LimbProxy()", "could not subscribe");
}

void LimbProxy::Unsubscribe(void)
{
  assert(mDevice!=NULL);
  scoped_lock_t lock(mPc->mMutex);
  playerc_limb_unsubscribe(mDevice);
  playerc_limb_destroy(mDevice);
  mDevice = NULL;
}

// interface that all proxies SHOULD provide
std::ostream&
std::operator << (std::ostream& os, const PlayerCc::LimbProxy& a)
{
  player_limb_data_t data = a.GetData ();
  player_limb_geom_req_t geom = a.GetGeom ();

  int old_precision = os.precision(3);
  std::_Ios_Fmtflags old_flags = os.flags();
  os.setf(std::ios::fixed);

  os << "Limb offset: " << geom.basePos.px << ", " << geom.basePos.py << ", " << geom.basePos.pz << endl;
  os << "End effector position: " << data.position.px << ", " << data.position.py << ", " << data.position.pz << endl;
  os << "Approach vector: " << data.approach.px << ", " << data.approach.py << ", " << data.approach.pz << endl;
  os << "Orientation vector: " << data.orientation.px << ", " << data.orientation.py << ", " << data.orientation.pz << endl;
  switch (data.state)
  {
    case PLAYER_LIMB_STATE_IDLE:
      os << "Limb is idle" << endl;
      break;
    case PLAYER_LIMB_STATE_BRAKED:
      os << "Limb is braked" << endl;
      break;
    case PLAYER_LIMB_STATE_MOVING:
      os << "Limb is moving" << endl;
      break;
    case PLAYER_LIMB_STATE_OOR:
      os << "Limb cannot reach requested pose" << endl;
      break;
    case PLAYER_LIMB_STATE_COLL:
      os << "Limb is obstructed by possible collision" << endl;
      break;
  }

  os.precision(old_precision);
  os.flags(old_flags);

  return os;
}

// Power control
void LimbProxy::SetPowerConfig(bool aVal)
{
  scoped_lock_t lock(mPc->mMutex);
  int ret = playerc_limb_power(mDevice, aVal ? 1 : 0);

  if (ret == -2)
    throw PlayerError("LimbProxy::SetPowerConfig", "NACK", ret);
  else if (ret != 0)
    throw PlayerError("LimbProxy::SetPowerConfig",
                      playerc_error_str(),
                      ret);
}

// Brakes control
void LimbProxy::SetBrakesConfig(bool aVal)
{
  scoped_lock_t lock(mPc->mMutex);
  int ret = playerc_limb_brakes(mDevice, aVal ? 1 : 0);

  if (ret == -2)
    throw PlayerError("LimbProxy::SetBrakesConfig", "NACK", ret);
  else if (ret != 0)
    throw PlayerError("LimbProxy::SetBrakesConfig",
                      playerc_error_str(),
                      ret);
}

// Speed config
void LimbProxy::SetSpeedConfig (float aSpeed)
{
  scoped_lock_t lock(mPc->mMutex);
  int ret = playerc_limb_speed_config(mDevice, aSpeed);

  if (ret == -2)
    throw PlayerError("LimbProxy::SetSpeedConfig", "NACK", ret);
  else if (ret != 0)
    throw PlayerError("LimbProxy::SetSpeedConfig",
                      playerc_error_str(),
                      ret);
}

// Move the limb to the home position
void LimbProxy::MoveHome(void)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_limb_home_cmd(mDevice);
}

// Stop the limb immediately
void LimbProxy::Stop(void)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_limb_stop_cmd(mDevice);
}

// Move the end effector to a given pose
void LimbProxy::SetPose(float aPX, float aPY, float aPZ,
                        float aAX, float aAY, float aAZ,
                        float aOX, float aOY, float aOZ)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_limb_setpose_cmd(mDevice, aPX, aPY, aPZ, aAX, aAY, aAZ, aOX, aOY, aOZ);
}

// Move the end effector to a given position, ignoring orientation
void LimbProxy::SetPosition(float aX, float aY, float aZ)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_limb_setposition_cmd(mDevice, aX, aY, aZ);
}

// Move the end effector along a vector of given length, maintaining current orientation
void LimbProxy::VectorMove(float aX, float aY, float aZ, float aLength)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_limb_vecmove_cmd(mDevice, aX, aY, aZ, aLength);
}

// Accessor method for getting the limb's data
player_limb_data_t LimbProxy::GetData(void) const
{
  return GetVar(mDevice->data);
}

// Same again for getting geometry
player_limb_geom_req_t LimbProxy::GetGeom(void) const
{
  return GetVar(mDevice->geom);
}

void LimbProxy::RequestGeometry(void)
{
  scoped_lock_t lock(mPc->mMutex);
  int ret = playerc_limb_get_geom(mDevice);

  if (ret == -2)
    throw PlayerError("LimbProxy::RequestGeometry", "NACK", ret);
  else if (ret != 0)
    throw PlayerError("LimbProxy::RequestGeometry",
                      playerc_error_str(),
                      ret);
}
