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
 * $Id: blobfinderproxy.cc 4227 2007-10-24 22:32:04Z thjc $
 *
 * client-side blobfinder device
 */

#include "playerc++.h"

using namespace PlayerCc;

BlobfinderProxy::BlobfinderProxy(PlayerClient *aPc, uint32_t aIndex)
  : ClientProxy(aPc, aIndex),
  mDevice(NULL)
{
  Subscribe(aIndex);
  // how can I get this into the clientproxy.cc?
  // right now, we're dependent on knowing its device type
  mInfo = &(mDevice->info);
}

BlobfinderProxy::~BlobfinderProxy()
{
  Unsubscribe();
}

void
BlobfinderProxy::Subscribe(uint32_t aIndex)
{
  scoped_lock_t lock(mPc->mMutex);
  mDevice = playerc_blobfinder_create(mClient, aIndex);
  if (NULL==mDevice)
    throw PlayerError("BlobfinderProxy::BlobfinderProxy()", "could not create");

  if (0 != playerc_blobfinder_subscribe(mDevice, PLAYER_OPEN_MODE))
    throw PlayerError("BlobfinderProxy::BlobfinderProxy()", "could not subscribe");
}

void
BlobfinderProxy::Unsubscribe()
{
  assert(NULL!=mDevice);
  scoped_lock_t lock(mPc->mMutex);
  playerc_blobfinder_unsubscribe(mDevice);
  playerc_blobfinder_destroy(mDevice);
  mDevice = NULL;
}

std::ostream& std::operator << (std::ostream &os, const PlayerCc::BlobfinderProxy &c)
{
  os << "#Blobfinder(" << c.GetInterface() << ":" << c.GetIndex() << ")" << std::endl;
  os << "Count:" << c.GetCount() << std::endl;
  for (unsigned int i=0;i < c.GetCount();i++)
  {
    os << "  blob " << i << ":" << std::endl;
    os << "               id: " << c.GetBlob(i).id << std::endl;
    os << "             area: " << c.GetBlob(i).area << std::endl;
    os << "                X: " << c.GetBlob(i).x << std::endl;
    os << "                Y: " << c.GetBlob(i).y << std::endl;
    os << "             Left: " << c.GetBlob(i).left << std::endl;
    os << "            Right: " << c.GetBlob(i).right << std::endl;
    os << "              Top: " << c.GetBlob(i).top << std::endl;
    os << "           Bottom: " << c.GetBlob(i).bottom << std::endl;
  }
  return os;
}


