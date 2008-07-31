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
 * $Id: fiducialproxy.cc 4227 2007-10-24 22:32:04Z thjc $
 */

#include "playerc++.h"

using namespace PlayerCc;

FiducialProxy::FiducialProxy(PlayerClient *aPc, uint32_t aIndex)
  : ClientProxy(aPc, aIndex),
  mDevice(NULL)
{
  Subscribe(aIndex);
  // how can I get this into the clientproxy.cc?
  // right now, we're dependent on knowing its device type
  mInfo = &(mDevice->info);
}

FiducialProxy::~FiducialProxy()
{
  Unsubscribe();
}

void
FiducialProxy::Subscribe(uint32_t aIndex)
{
  scoped_lock_t lock(mPc->mMutex);
  mDevice = playerc_fiducial_create(mClient, aIndex);
  if (NULL==mDevice)
    throw PlayerError("FiducialProxy::FiducialProxy()", "could not create");

  if (0 != playerc_fiducial_subscribe(mDevice, PLAYER_OPEN_MODE))
    throw PlayerError("FiducialProxy::FiducialProxy()", "could not subscribe");
}

void
FiducialProxy::Unsubscribe()
{
  assert(NULL!=mDevice);
  scoped_lock_t lock(mPc->mMutex);
  playerc_fiducial_unsubscribe(mDevice);
  playerc_fiducial_destroy(mDevice);
  mDevice = NULL;
}



std::ostream& std::operator << (std::ostream &os, const PlayerCc::FiducialProxy &c)
{
  os << "#Fiducial (" << c.GetInterface() << ":" << c.GetIndex() << ")" << std::endl;
  os << c.GetCount() << std::endl;
  for (unsigned int i=0;i < c.GetCount();i++)
  {
    player_fiducial_item_t item = c.GetFiducialItem(i);
    os << "  " << i << " - " << item.id << ": "
       << item.pose << " " << item.upose << std::endl;
  }
  return os;
}

// Get the fiducial geometry.  The writes the result into the proxy
// rather than returning it to the caller.
void
FiducialProxy::RequestGeometry()
{
  scoped_lock_t lock(mPc->mMutex);
  if (0 != playerc_fiducial_get_geom(mDevice))
    throw PlayerError("FiducialProxy::RequestGeometry()", "error getting geom");
  return;
}

