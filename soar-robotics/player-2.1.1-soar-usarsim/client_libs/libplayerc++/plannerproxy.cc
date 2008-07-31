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
 * $Id: plannerproxy.cc 4435 2008-03-23 09:02:12Z thjc $
 */

#include "playerc++.h"

using namespace PlayerCc;

PlannerProxy::PlannerProxy(PlayerClient *aPc, uint32_t aIndex)
  : ClientProxy(aPc, aIndex),
  mDevice(NULL)
{
  Subscribe(aIndex);
  // how can I get this into the clientproxy.cc?
  // right now, we're dependent on knowing its device type
  mInfo = &(mDevice->info);
}

PlannerProxy::~PlannerProxy()
{
  Unsubscribe();
}

void
PlannerProxy::Subscribe(uint32_t aIndex)
{
  scoped_lock_t lock(mPc->mMutex);
  mDevice = playerc_planner_create(mClient, aIndex);

  if (NULL==mDevice)
    throw PlayerError("PlannerProxy::PlannerProxy()", "could not create");

  if (0 != playerc_planner_subscribe(mDevice, PLAYER_OPEN_MODE))
    throw PlayerError("PlannerProxy::PlannerProxy()", "could not subscribe");
}

void
PlannerProxy::Unsubscribe()
{
  assert(NULL!=mDevice);
  scoped_lock_t lock(mPc->mMutex);
  playerc_planner_unsubscribe(mDevice);
  playerc_planner_destroy(mDevice);
  mDevice = NULL;
}

std::ostream&
std::operator << (std::ostream &os, const PlayerCc::PlannerProxy &c)
{
  player_pose2d_t p;
  os << "#Planner (" << c.GetInterface() << ":" << c.GetIndex() << ")" << std::endl;
  os << "#xpos\typos\ttheta\t\txgoal\tygoal\tthetagoal\tvalid\tdone" << std::endl;

  p = c.GetPose();
  os << p.px << " " << p.py << " " << p.pa << " \t";
  
  p = c.GetGoal();
  os << p.px << " " << p.py << " " << p.pa << " \t";
  
  os << c.GetPathValid() << "\t" << c.GetPathDone() << std::endl;

  return os;
}

void
PlannerProxy::SetGoalPose(double aGx, double aGy, double aGa)
{
  scoped_lock_t lock(mPc->mMutex);
  if (0 != playerc_planner_set_cmd_pose(mDevice, aGx, aGy, aGa))
    throw PlayerError("PlannerProxy::SetGoalPose()", "error setting goal");
  return;
}

void
PlannerProxy::RequestWaypoints()
{
  scoped_lock_t lock(mPc->mMutex);
  if (0 != playerc_planner_get_waypoints(mDevice))
    throw PlayerError("PlannerProxy::RequestWaypoints()",
                      "error requesting waypoint");
  return;
}


void
PlannerProxy::SetEnable(bool aEnable)
{
  scoped_lock_t lock(mPc->mMutex);
  if (0 != playerc_planner_enable(mDevice, aEnable))
    throw PlayerError("PlannerProxy::SetEnable()", "error setting enable");
  return;
}

/// Waypoint[i] location (m)
double PlannerProxy::GetIx(int i) const 
{ 
  if (i < mDevice->waypoint_count)
    return GetVar(mDevice->waypoints[i][0]); 
  else
    throw PlayerError("PlannerProxy::GetIx()", "invalid index");
    return 0;
}

/// Waypoint[i] location (m)
double PlannerProxy::GetIy(int i) const 
{ 
  if (i < mDevice->waypoint_count)
    return GetVar(mDevice->waypoints[i][1]); 
  else
    throw PlayerError("PlannerProxy::GetIx()", "invalid index");
    return 0;
}

/// Waypoint[i] location (m)
double PlannerProxy::GetIa(int i) const
{ 
  if (i < mDevice->waypoint_count)
    return GetVar(mDevice->waypoints[i][2]); 
  else
    throw PlayerError("PlannerProxy::GetIx()", "invalid index");
    return 0;
}
