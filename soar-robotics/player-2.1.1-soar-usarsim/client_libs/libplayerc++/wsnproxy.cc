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
 * Author: Radu Bogdan Rusu
 * client-side WSN device 
 */

#include "playerc++.h"
#include <cstring>
#include <cstdio>
#include <cmath>
#include <climits>

using namespace PlayerCc;

WSNProxy::WSNProxy(PlayerClient *aPc, uint32_t aIndex)
	: ClientProxy(aPc, aIndex),
  mDevice(NULL)
{
    Subscribe(aIndex);
  // how can I get this into the clientproxy.cc?
  // right now, we're dependent on knowing its device type
    mInfo = &(mDevice->info);
}

WSNProxy::~WSNProxy()
{
    Unsubscribe();
}

void
WSNProxy::Subscribe(uint32_t aIndex)
{
    scoped_lock_t lock(mPc->mMutex);
    mDevice = playerc_wsn_create(mClient, aIndex);
    if (NULL==mDevice)
	throw PlayerError("WSNProxy::WSNProxy()", "could not create");

    if (0 != playerc_wsn_subscribe(mDevice, PLAYER_OPEN_MODE))
	throw PlayerError("WSNProxy::WSNProxy()", "could not subscribe");
}

void
WSNProxy::Unsubscribe()
{
    assert(NULL!=mDevice);
    scoped_lock_t lock(mPc->mMutex);
    playerc_wsn_unsubscribe(mDevice);
    playerc_wsn_destroy(mDevice);
    mDevice = NULL;
}

std::ostream& std::operator << (std::ostream &os, const PlayerCc::WSNProxy &c)
{
    player_wsn_node_data_t cdp = c.GetNodeDataPacket();
    os << "#WSN(" << c.GetInterface() << ":" << c.GetIndex() << ")" << std::endl;
    os << "Node type: " << c.GetNodeType() << " with ID " << c.GetNodeID()
       << " and parent " << c.GetNodeParentID() << " holds:" << std::endl;
    os << "   accel_{X,Y,Z}  : [" << cdp.accel_x << "," << cdp.accel_y << ","
       << cdp.accel_z << "]" << std::endl;
    os << "   magn_{X,Y,Z}   : [" << cdp.magn_x << "," << cdp.magn_y << ","
       << cdp.magn_z << "]" << std::endl;
    os << "   temperature    : [" << cdp.temperature << "]" << std::endl;
    os << "   light          : [" << cdp.light       << "]" << std::endl;
    os << "   microphone     : [" << cdp.mic         << "]" << std::endl;
    os << "   battery voltage: [" << cdp.battery     << "]" << std::endl;
    return os;
}

void
WSNProxy::SetDevState(int nodeID, int groupID, int devNr, int value)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_wsn_set_devstate(mDevice, nodeID, groupID, devNr, value);
}

void
WSNProxy::Power(int nodeID, int groupID, int value)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_wsn_power(mDevice, nodeID, groupID, value);
}

void
WSNProxy::DataType(int value)
{
  playerc_wsn_datatype(mDevice, value);
}

void
WSNProxy::DataFreq(int nodeID, int groupID, float frequency)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_wsn_datafreq(mDevice, nodeID, groupID, frequency);
}
