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

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "playerc++.h"

using namespace PlayerCc;

RangerProxy::RangerProxy(PlayerClient *aPc, uint32_t aIndex)
  : ClientProxy(aPc, aIndex),
  mDevice(NULL)
{
  Subscribe(aIndex);
  mInfo = &(mDevice->info);
}

RangerProxy::~RangerProxy()
{
  Unsubscribe();
}

void RangerProxy::Subscribe(uint32_t aIndex)
{
  scoped_lock_t lock(mPc->mMutex);
  mDevice = playerc_ranger_create(mClient, aIndex);
  if (mDevice == NULL)
    throw PlayerError("RangerProxy::RangerProxy()", "could not create");

  if (playerc_ranger_subscribe(mDevice, PLAYER_OPEN_MODE) != 0)
    throw PlayerError("RangerProxy::RangerProxy()", "could not subscribe");
}

void RangerProxy::Unsubscribe()
{
  assert(mDevice != NULL);
  scoped_lock_t lock(mPc->mMutex);
  playerc_ranger_unsubscribe(mDevice);
  playerc_ranger_destroy(mDevice);
  mDevice = NULL;
}

player_pose3d_t RangerProxy::GetSensorPose(uint32_t aIndex) const
{
  if (aIndex > mDevice->sensor_count)
    throw PlayerError("RangerProxy::GetSensorPose", "index out of bounds");
  return GetVar(mDevice->sensor_poses[aIndex]);
}

player_bbox3d_t RangerProxy::GetSensorSize(uint32_t aIndex) const
{
  if (aIndex > mDevice->sensor_count)
    throw PlayerError("RangerProxy::GetSensorSize", "index out of bounds");
  return GetVar(mDevice->sensor_sizes[aIndex]);
}

double RangerProxy::GetRange(uint32_t aIndex) const
{
  if (aIndex > mDevice->ranges_count)
    throw PlayerError("RangerProxy::GetRange", "index out of bounds");
  return GetVar(mDevice->ranges[aIndex]);
}

double RangerProxy::GetIntensity(uint32_t aIndex) const
{
  if (aIndex > mDevice->intensities_count)
    throw PlayerError("RangerProxy::GetIntensity", "index out of bounds");
  return GetVar(mDevice->intensities[aIndex]);
}

void RangerProxy::SetPower(bool aEnable)
{
  scoped_lock_t lock(mPc->mMutex);
  if (playerc_ranger_power_config(mDevice, aEnable ? 1 : 0) != 0)
    throw PlayerError("RangerProxy::SetPower()", "error setting power");
}

void RangerProxy::SetIntensityData(bool aEnable)
{
  scoped_lock_t lock(mPc->mMutex);
  if (playerc_ranger_intns_config(mDevice, aEnable ? 1 : 0) != 0)
    throw PlayerError("RangerProxy::SetIntensityData()", "error setting power");
}

void RangerProxy::RequestGeom()
{
  scoped_lock_t lock(mPc->mMutex);
  if (playerc_ranger_get_geom(mDevice) != 0)
    throw PlayerError("RangerProxy::RequestGeom()", "error requesting geometry");
}

void RangerProxy::Configure(double aMinAngle, double aMaxAngle, double aResolution,
                            double aMaxRange, double aRangeRes, double aFrequency)
{
  scoped_lock_t lock(mPc->mMutex);
  if (0 != playerc_ranger_set_config(mDevice, aMinAngle, aMaxAngle, aResolution,
                                     aMaxRange, aRangeRes, aFrequency))
    throw PlayerError("RangerProxy::Configure()", "error setting config");
}

void RangerProxy::RequestConfigure()
{
  scoped_lock_t lock(mPc->mMutex);
  if (0 != playerc_ranger_get_config(mDevice, NULL, NULL, NULL, NULL, NULL, NULL))
    throw PlayerError("RangerProxy::RequestConfigure()", "error getting config");
}


std::ostream& std::operator << (std::ostream &os, const PlayerCc::RangerProxy &c)
{
  player_pose3d_t pose;
  player_bbox3d_t size;

  os << "#Ranger (" << c.GetInterface() << ":" << c.GetIndex() << ")" << std::endl;
  pose = c.GetDevicePose ();
  os << "Device pose: (" << pose.px << ", " << pose.py << ", " << pose.pz << "), (" <<
        pose.proll << ", " << pose.ppitch << ", " << pose.pyaw << ")" << endl;
  size = c.GetDeviceSize ();
  os << "Device size: (" << size.sw << ", " << size.sl << ", " << size.sh << ")" << endl;
  if (c.GetSensorCount() > 0)
  {
    os << c.GetSensorCount() << " sensors:" << endl;
    for (uint32_t ii = 0; ii < c.GetSensorCount(); ii++)
    {
      pose = c.GetSensorPose(ii);
      size = c.GetSensorSize(ii);
      os << "  Pose: (" << pose.px << ", " << pose.py << ", " << pose.pz << "), (" <<
         pose.proll << ", " << pose.ppitch << ", " << pose.pyaw << ")" << "\tSize: (" <<
         size.sw << ", " << size.sl << ", " << size.sh << ")" << endl;
    }
  }
  if (c.GetRangeCount() > 0)
  {
    os << c.GetRangeCount() << " range readings:" << endl << "  [";
    uint32_t ii;
    for (ii = 0; ii < c.GetRangeCount() - 1; ii++)
      os << c.GetRange(ii) << ", ";
    os << c.GetRange(ii) << "]" << endl;
  }
  if (c.GetIntensityCount() > 0)
  {
    os << c.GetIntensityCount() << " intensity readings:" << endl << "  [";
    uint32_t ii;
    for (ii = 0; ii < c.GetIntensityCount() - 1; ii++)
      os << c.GetIntensity(ii) << ", ";
    os << c.GetIntensity(ii) << "]" << endl;
  }

  return os;
}
