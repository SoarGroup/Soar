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
 * $Id: actarrayproxy.cc 4323 2008-01-08 03:53:38Z thjc $
 *
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <cassert>
#include <sstream>
#include <iomanip>
#include <vector>

#include "playerc++.h"
#include "debug.h"

using namespace PlayerCc;

ActArrayProxy::ActArrayProxy(PlayerClient *aPc, uint32_t aIndex)
  : ClientProxy(aPc, aIndex),
  mDevice(NULL)
{
  Subscribe(aIndex);
  mInfo = &(mDevice->info);
}

ActArrayProxy::~ActArrayProxy()
{
  Unsubscribe();
}

void ActArrayProxy::Subscribe(uint32_t aIndex)
{
  scoped_lock_t lock(mPc->mMutex);
  mDevice = playerc_actarray_create(mClient, aIndex);
  if (mDevice==NULL)
    throw PlayerError("ActArrayProxy::ActArrayProxy()", "could not create");

  if (playerc_actarray_subscribe(mDevice, PLAYER_OPEN_MODE) != 0)
    throw PlayerError("ActArrayProxy::ActArrayProxy()", "could not subscribe");
}

void ActArrayProxy::Unsubscribe(void)
{
  assert(mDevice!=NULL);
  scoped_lock_t lock(mPc->mMutex);
  playerc_actarray_unsubscribe(mDevice);
  playerc_actarray_destroy(mDevice);
  mDevice = NULL;
}

// interface that all proxies SHOULD provide
std::ostream& std::operator << (std::ostream& os, const PlayerCc::ActArrayProxy& a)
{
  player_actarray_actuator_t data;
  player_actarray_actuatorgeom_t geom;

  int old_precision = os.precision(3);
  std::_Ios_Fmtflags old_flags = os.flags();
  os.setf(std::ios::fixed);

  os << a.GetCount () << " actuators:" << std::endl;
  os << "Act \tType\tLength\tOrientation\t\tAxis\t\t\tMin\tCentre\tMax\tHome"
        "\tCfgSpd\tPos\tSpeed\tState\tBrakes" << std::endl;
  for (uint32_t ii = 0; ii < a.GetCount (); ii++)
  {
    data = a.GetActuatorData(ii);
    geom = a.GetActuatorGeom(ii);
    os <<  ii << '\t'
       << (geom.type ? "Linear" : "Rotary") << '\t'
       << geom.length << '\t'
       << "(" << geom.orientation.proll << ", " << geom.orientation.ppitch << ", " << geom.orientation.pyaw << ")\t"
       << "(" << geom.axis.px << ", " << geom.axis.py << ", " << geom.axis.pz << ")\t"
       << geom.min << '\t'
       << geom.centre << '\t'
       << geom.max << '\t'
       << geom.home << '\t'
       << geom.config_speed << '\t'
       << data.position << '\t'
       << data.speed << '\t'
       << static_cast<int> (data.state)
       << '\t' << (geom.hasbrakes ? "Y" : "N")
       << std::endl;
  }
  os << "(" << a.GetBasePos().px << ", " << a.GetBasePos().py << ", " << a.GetBasePos().pz << ")" << std::endl;
  os << "(" << a.GetBaseOrientation().proll << ", " << a.GetBaseOrientation().ppitch << ", " << a.GetBaseOrientation().pyaw << ")" << std::endl;

  os.precision(old_precision);
  os.flags(old_flags);

  return os;
}

// Power control
void ActArrayProxy::SetPowerConfig(bool aVal)
{
  scoped_lock_t lock(mPc->mMutex);
  int ret = playerc_actarray_power(mDevice, aVal ? 1 : 0);

  if (ret == -2)
    throw PlayerError("ActArrayProxy::SetPowerConfig", "NACK", ret);
  else if (ret != 0)
    throw PlayerError("ActArrayProxy::SetPowerConfig",
                      playerc_error_str(),
                      ret);
}

// Brakes control
void ActArrayProxy::SetBrakesConfig(bool aVal)
{
  scoped_lock_t lock(mPc->mMutex);
  int ret = playerc_actarray_brakes(mDevice, aVal ? 1 : 0);

  if (ret == -2)
    throw PlayerError("ActArrayProxy::SetBrakesConfig", "NACK", ret);
  else if (ret != 0)
    throw PlayerError("ActArrayProxy::SetBrakesConfig",
                      playerc_error_str(),
                      ret);
}

// Speed config
void ActArrayProxy::SetSpeedConfig (uint32_t aJoint, float aSpeed)
{
  scoped_lock_t lock(mPc->mMutex);
  int ret = playerc_actarray_speed_config(mDevice, aJoint, aSpeed);

  if (ret == -2)
    throw PlayerError("ActArrayProxy::SetSpeedConfig", "NACK", ret);
  else if (ret != 0)
    throw PlayerError("ActArrayProxy::SetSpeedConfig",
                      playerc_error_str(),
                      ret);
}

// Send an actuator to a position
void ActArrayProxy::MoveTo(uint32_t aJoint, float aPosition)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_actarray_position_cmd(mDevice, aJoint, aPosition);
}

// Send an actuator to a position
void ActArrayProxy::MoveToMulti(std::vector<float> aPosition)
{
  float * values = new float[aPosition.size()];
  unsigned int i = 0;
  for (std::vector<float>::const_iterator itr = aPosition.begin(); itr != aPosition.end(); ++itr)
    values[i++] = *itr;
  scoped_lock_t lock(mPc->mMutex);
  playerc_actarray_multi_position_cmd(mDevice, values, aPosition.size());
  delete [] values;
}


// Move an actuator at a speed
void ActArrayProxy::MoveAtSpeed(uint32_t aJoint, float aSpeed)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_actarray_speed_cmd(mDevice, aJoint, aSpeed);
}

// Send an actuator to a position
void ActArrayProxy::MoveAtSpeedMulti(std::vector<float> aSpeed)
{
  float * values = new float[aSpeed.size()];
  unsigned int i = 0;
  for (std::vector<float>::const_iterator itr = aSpeed.begin(); itr != aSpeed.end(); ++itr)
    values[i++] = *itr;
  scoped_lock_t lock(mPc->mMutex);
  playerc_actarray_multi_speed_cmd(mDevice, values, aSpeed.size());
  delete [] values;
}

// Send an actuator, or all actuators, home
void ActArrayProxy::MoveHome(int aJoint)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_actarray_home_cmd(mDevice, aJoint);
}

// Move an actuator at a speed
void ActArrayProxy::SetActuatorCurrent(uint32_t aJoint, float aCurrent)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_actarray_current_cmd(mDevice, aJoint, aCurrent);
}

// Send an actuator to a position
void ActArrayProxy::SetActuatorCurrentMulti(std::vector<float> aCurrent)
{
  float * values = new float[aCurrent.size()];
  unsigned int i = 0;
  for (std::vector<float>::const_iterator itr = aCurrent.begin(); itr != aCurrent.end(); ++itr)
    values[i++] = *itr;
  scoped_lock_t lock(mPc->mMutex);
  playerc_actarray_multi_current_cmd(mDevice, values, aCurrent.size());
  delete [] values;
}

player_actarray_actuator_t ActArrayProxy::GetActuatorData(uint32_t aJoint) const
{
  if (aJoint > mDevice->actuators_count)
  {
    player_actarray_actuator_t empty;
    memset(&empty, 0, sizeof(player_actarray_actuator_t));
    return empty;
  }
  else
    return GetVar(mDevice->actuators_data[aJoint]);
}

// Same again for getting actuator geometry
player_actarray_actuatorgeom_t ActArrayProxy::GetActuatorGeom(uint32_t aJoint) const
{
  if (mDevice->actuators_geom == NULL || aJoint > mDevice->actuators_count)
  {
    player_actarray_actuatorgeom_t empty;
    memset(&empty, 0, sizeof(player_actarray_actuatorgeom_t));
    return empty;
  }
  else
    return GetVar(mDevice->actuators_geom[aJoint]);
}

void ActArrayProxy::RequestGeometry(void)
{
  scoped_lock_t lock(mPc->mMutex);
  int ret = playerc_actarray_get_geom(mDevice);

  if (ret == -2)
    throw PlayerError("ActArrayProxy::RequestGeometry", "NACK", ret);
  else if (ret != 0)
    throw PlayerError("ActArrayProxy::RequestGeometry",
                      playerc_error_str(),
                      ret);
}
