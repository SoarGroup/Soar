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
 * $Id: opaqueproxy.cc 4297 2007-12-10 04:44:15Z thjc $
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

OpaqueProxy::OpaqueProxy(PlayerClient *aPc, uint32_t aIndex)
  : ClientProxy(aPc, aIndex),
  mDevice(NULL)
{
  Subscribe(aIndex);
  // how can I get this into the clientproxy.cc?
  // right now, we're dependent on knowing its device type
  mInfo = &(mDevice->info);
}

OpaqueProxy::~OpaqueProxy()
{
  Unsubscribe();
}

void
OpaqueProxy::Subscribe(uint32_t aIndex)
{
  scoped_lock_t lock(mPc->mMutex);
  mDevice = playerc_opaque_create(mClient, aIndex);
  if (NULL==mDevice)
    throw PlayerError("OpaqueProxy::OpaqueProxy()", "could not create");

  if (0 != playerc_opaque_subscribe(mDevice, PLAYER_OPEN_MODE))
    throw PlayerError("OpaqueProxy::OpaqueProxy()", "could not subscribe");
}

void
OpaqueProxy::SendCmd(player_opaque_data_t* aData)
{
  playerc_opaque_cmd(mDevice, aData);
}

int
OpaqueProxy::SendReq(player_opaque_data_t* aRequest)
{
  player_opaque_data_t *aReply;
  int result = playerc_opaque_req(mDevice, aRequest, &aReply);
  if (result == 0)
  {
    memcpy(mDevice->data, aReply->data, aReply->data_count);
    mDevice->data_count = aReply->data_count;
  }
  player_opaque_data_t_free(aReply);
  return result;
}

void
OpaqueProxy::Unsubscribe()
{
  assert(NULL!=mDevice);
  scoped_lock_t lock(mPc->mMutex);
  playerc_opaque_unsubscribe(mDevice);
  playerc_opaque_destroy(mDevice);
  mDevice = NULL;
}

std::ostream& std::operator << (std::ostream& os, const PlayerCc::OpaqueProxy& c)
{
	os << "Count is: "<< c.GetCount() << "Data:" << endl;
	uint8_t * data;
	data = new uint8_t[4096];
	c.GetData(data);
	for(unsigned int i = 0; i < c.GetCount(); i++)
	{
		os << hex << setw(2) << setfill('0') << static_cast<unsigned int> (data[i]);
	}
  return os;
}
