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
 * $Id: aioproxy.cc 4227 2007-10-24 22:32:04Z thjc $
 */


#include "playerc++.h"

using namespace PlayerCc;

AioProxy::AioProxy(PlayerClient *aPc, uint32_t aIndex)
  : ClientProxy(aPc, aIndex),
  mDevice(NULL)
{
  Subscribe(aIndex);
  // how can I get this into the clientproxy.cc?
  // right now, we're dependent on knowing its device type
  mInfo = &(mDevice->info);
}

AioProxy::~AioProxy()
{
  Unsubscribe();
}

void
AioProxy::Subscribe(uint32_t aIndex)
{
  scoped_lock_t lock(mPc->mMutex);
  mDevice = playerc_aio_create(mClient, aIndex);
  if (NULL==mDevice)
    throw PlayerError("AioProxy::AioProxy()", "could not create");

  if (0 != playerc_aio_subscribe(mDevice, PLAYER_OPEN_MODE))
    throw PlayerError("AioProxy::AioProxy()", "could not subscribe");
}

void
AioProxy::Unsubscribe()
{
  assert(NULL!=mDevice);
  scoped_lock_t lock(mPc->mMutex);
  playerc_aio_unsubscribe(mDevice);
  playerc_aio_destroy(mDevice);
  mDevice = NULL;
}

void
AioProxy::SetVoltage(uint32_t aIndex, double aVoltage)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_aio_set_output(mDevice, aIndex, aVoltage);
}

std::ostream&
std::operator << (std::ostream &os, const PlayerCc::AioProxy &c)
{
  os << "#AIO (" << c.GetInterface() << ":" << c.GetIndex() << ")" << std::endl;
  //os << c.GetCount() << std::endl;
  if (c.GetCount() < 0)
  {
    os << "WARNING: AioProxy received a negative count value.\n" << std::endl;
  }
  else
  {
    for (uint32_t i=0; i < c.GetCount(); ++i)
    {
      os << "AIO" << i << " - " << c[i] << std::endl;
    }
  }

  return os;
}
