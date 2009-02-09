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
 * $Id: graphics3dproxy.cc 4227 2007-10-24 22:32:04Z thjc $
 */

#include "playerc++.h"

using namespace PlayerCc;

Graphics3dProxy::Graphics3dProxy(PlayerClient *aPc, uint32_t aIndex)
  : ClientProxy(aPc, aIndex),
  mDevice(NULL)
{
  Subscribe(aIndex);
  // how can I get this into the clientproxy.cc?
  // right now, we're dependent on knowing its device type
  mInfo = &(mDevice->info);
}

Graphics3dProxy::~Graphics3dProxy()
{
  Unsubscribe();
}

void
Graphics3dProxy::Subscribe(uint32_t aIndex)
{
  scoped_lock_t lock(mPc->mMutex);
  mDevice = playerc_graphics3d_create(mClient, aIndex);
  if (NULL==mDevice)
    throw PlayerError("Graphics3dProxy::Graphics3dProxy()", "could not create");

  if (0 != playerc_graphics3d_subscribe(mDevice, PLAYER_OPEN_MODE))
    throw PlayerError("Graphics3dProxy::Graphics3dProxy()", "could not subscribe");
}

void
Graphics3dProxy::Unsubscribe()
{
  assert(NULL!=mDevice);
  scoped_lock_t lock(mPc->mMutex);
  playerc_graphics3d_unsubscribe(mDevice);
  playerc_graphics3d_destroy(mDevice);
  mDevice = NULL;
}


void
Graphics3dProxy::Clear( void )
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_graphics3d_clear(mDevice); 
}

void
Graphics3dProxy::Draw(player_graphics3d_draw_mode_t mode, player_point_3d_t pts[], int count)
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_graphics3d_draw(mDevice,mode,pts,count); 
}
 
void
Graphics3dProxy::Color( player_color_t col )
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_graphics3d_setcolor(mDevice, col); 
}

void
Graphics3dProxy::Color( uint8_t red,  uint8_t green,  uint8_t blue,  uint8_t alpha )
{
  player_color_t col;
  col.red = red;
  col.green = green;
  col.blue = blue;
  col.alpha = alpha;

  this->Color( col );
}
