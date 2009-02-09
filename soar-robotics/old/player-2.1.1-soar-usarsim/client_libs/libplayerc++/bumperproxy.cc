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
 * $Id: bumperproxy.cc 4232 2007-11-01 22:16:23Z gerkey $
 */

#include "playerc++.h"

using namespace PlayerCc;

BumperProxy::BumperProxy(PlayerClient *aPc, uint32_t aIndex)
  : ClientProxy(aPc, aIndex),
  mDevice(NULL)
{
  Subscribe(aIndex);
  // how can I get this into the clientproxy.cc?
  // right now, we're dependent on knowing its device type
  mInfo = &(mDevice->info);
}

BumperProxy::~BumperProxy()
{
  Unsubscribe();
}

void
BumperProxy::Subscribe(uint32_t aIndex)
{
  scoped_lock_t lock(mPc->mMutex);
  mDevice = playerc_bumper_create(mClient, aIndex);
  if (NULL==mDevice)
    throw PlayerError("BumperProxy::BumperProxy()", "could not create");

  if (0 != playerc_bumper_subscribe(mDevice, PLAYER_OPEN_MODE))
    throw PlayerError("BumperProxy::BumperProxy()", "could not subscribe");
}

void
BumperProxy::Unsubscribe()
{
  assert(NULL!=mDevice);
  scoped_lock_t lock(mPc->mMutex);
  playerc_bumper_unsubscribe(mDevice);
  playerc_bumper_destroy(mDevice);
  mDevice = NULL;
}

std::ostream& std::operator << (std::ostream &os, const PlayerCc::BumperProxy &c)
{
  os << "#Bumper(" << c.GetInterface() << ":" << c.GetIndex() << ")" << std::endl;
  os << "Count:" << c.GetCount() << std::endl;
  for (unsigned int i=0;i < c.GetCount();i++)
  {
    os << (c.IsBumped(i) ? '1' : '0');
  }
  return os;
}

bool
BumperProxy::IsAnyBumped()
{
  for (uint32_t i=0; i < GetCount(); ++i)
  {
    if (IsBumped(i))
      return true;
  }
  // if we got this far...
  return false;
}

void
BumperProxy::RequestBumperConfig()
{
  scoped_lock_t lock(mPc->mMutex);
  if (0 != playerc_bumper_get_geom(mDevice))
    throw PlayerError("BumperProxy::RequestBumperConfig()", "error getting geom");
  return;
}
