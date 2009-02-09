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
 * client-side RFID device 
 */

#include "playerc++.h"
#include <cstring>
#include <cstdio>
#include <cmath>
#include <climits>

using namespace PlayerCc;

RFIDProxy::RFIDProxy(PlayerClient *aPc, uint32_t aIndex)
	: ClientProxy(aPc, aIndex),
  mDevice(NULL)
{
	Subscribe(aIndex);
  // how can I get this into the clientproxy.cc?
  // right now, we're dependent on knowing its device type
	mInfo = &(mDevice->info);
}

RFIDProxy::~RFIDProxy()
{
	Unsubscribe();
}

void
RFIDProxy::Subscribe(uint32_t aIndex)
{
	scoped_lock_t lock(mPc->mMutex);
	mDevice = playerc_rfid_create(mClient, aIndex);
	if (NULL==mDevice)
		throw PlayerError("RFIDProxy::RFIDProxy()", "could not create");

	if (0 != playerc_rfid_subscribe(mDevice, PLAYER_OPEN_MODE))
		throw PlayerError("RFIDProxy::RFIDProxy()", "could not subscribe");
}

void
RFIDProxy::Unsubscribe()
{
	assert(NULL!=mDevice);
	scoped_lock_t lock(mPc->mMutex);
	playerc_rfid_unsubscribe(mDevice);
	playerc_rfid_destroy(mDevice);
	mDevice = NULL;
}

std::ostream& std::operator << (std::ostream &os, const PlayerCc::RFIDProxy &c)
{
	os << "#RFID(" << c.GetInterface() << ":" << c.GetIndex() << ")" << std::endl;
	os << "Tags count:" << c.GetTagsCount() << std::endl;
	for (uint32_t i=0;i < c.GetTagsCount();i++)
	{
		playerc_rfidtag_t tag;
		tag = c.GetRFIDTag(i);
		char RFIDtag[24];
		sprintf (RFIDtag, "$%2x$%2x$%2x$%2x$%2x$%2x$%2x$%2x", 
			 tag.guid[0], tag.guid[1], tag.guid[2], tag.guid[3],
			 tag.guid[4], tag.guid[5], tag.guid[6], tag.guid[7]);
		
		os << "  tag " << i << ":" << std::endl;
		os << "             type: " << tag.type << std::endl;
		os << "             guid: " << RFIDtag << std::endl;
	}
	return os;
}
