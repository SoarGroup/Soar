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
 * $Id: gripperproxy.cc 4227 2007-10-24 22:32:04Z thjc $
 */

#include "playerc++.h"

using namespace PlayerCc;

GripperProxy::GripperProxy(PlayerClient *aPc, uint32_t aIndex)
  : ClientProxy(aPc, aIndex),
  mDevice(NULL)
{
  Subscribe(aIndex);
  // how can I get this into the clientproxy.cc?
  // right now, we're dependent on knowing its device type
  mInfo = &(mDevice->info);
}

GripperProxy::~GripperProxy()
{
  Unsubscribe();
}

void
GripperProxy::Subscribe(uint32_t aIndex)
{
  scoped_lock_t lock(mPc->mMutex);
  mDevice = playerc_gripper_create(mClient, aIndex);
  if (NULL==mDevice)
    throw PlayerError("GripperProxy::GripperProxy()", "could not create");

  if (0 != playerc_gripper_subscribe(mDevice, PLAYER_OPEN_MODE))
    throw PlayerError("GripperProxy::GripperProxy()", "could not subscribe");
}

void
GripperProxy::Unsubscribe()
{
  assert(NULL!=mDevice);
  scoped_lock_t lock(mPc->mMutex);
  playerc_gripper_unsubscribe(mDevice);
  playerc_gripper_destroy(mDevice);
  mDevice = NULL;
}



std::ostream& std::operator << (std::ostream &os, const PlayerCc::GripperProxy &c)
{
  os << "#Gripper (" << c.GetInterface() << ":" << c.GetIndex() << ")" << std::endl;
  os << (c.GetState() == PLAYER_GRIPPER_STATE_OPEN ? "open" :
    (c.GetState() == PLAYER_GRIPPER_STATE_CLOSED ? "closed" :
      (c.GetState() == PLAYER_GRIPPER_STATE_MOVING ? "moving" : "error"))) << ", ";
  os << c.GetBeams() << " beams, storage " << c.GetCapacity () << "/" << c.GetStored () << ", ";
  os << "Pose: (" << c.GetPose().px << ", " << c.GetPose().py << ", " << c.GetPose().pz << "), (";
  os << c.GetPose().proll << ", " << c.GetPose().ppitch << ", " << c.GetPose().pyaw << ") ";
  os << "Outer size: (" << c.GetOuterSize().sw << ", " << c.GetOuterSize().sl << ", " << c.GetOuterSize().sh << ") ";
  os << "Inner size: (" << c.GetInnerSize().sw << ", " << c.GetInnerSize().sl << ", " << c.GetInnerSize().sh << ") ";
  os << std::endl;

  return os;
}

void GripperProxy::RequestGeometry()
{
  scoped_lock_t lock(mPc->mMutex);
  if (playerc_gripper_get_geom(mDevice) != 0)
    throw PlayerError("GripperProxy::RequestGeometry()", "error getting geometry");
  return;
}

// Send the open command
void GripperProxy::Open()
{
  scoped_lock_t lock(mPc->mMutex);
  if (playerc_gripper_open_cmd(mDevice) != 0)
    throw PlayerError("GripperProxy::Open()", "error sending open command");
  return;
}

// Send the close command
void GripperProxy::Close()
{
  scoped_lock_t lock(mPc->mMutex);
  if (playerc_gripper_close_cmd(mDevice) != 0)
    throw PlayerError("GripperProxy::Close()", "error sending close command");
  return;
}

// Send the stop command
void GripperProxy::Stop()
{
  scoped_lock_t lock(mPc->mMutex);
  if (playerc_gripper_stop_cmd(mDevice) != 0)
    throw PlayerError("GripperProxy::Stop()", "error sending stop command");
  return;
}

// Send the store command
void GripperProxy::Store()
{
  scoped_lock_t lock(mPc->mMutex);
  if (playerc_gripper_store_cmd(mDevice) != 0)
    throw PlayerError("GripperProxy::Store()", "error sending store command");
  return;
}

// Send the retrieve command
void GripperProxy::Retrieve()
{
  scoped_lock_t lock(mPc->mMutex);
  if (playerc_gripper_retrieve_cmd(mDevice) != 0)
    throw PlayerError("GripperProxy::Retrieve()", "error sending retrieve command");
  return;
}
