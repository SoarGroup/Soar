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
 * $Id: graphics2dproxy.cc 4227 2007-10-24 22:32:04Z thjc $
 */

#include "playerc++.h"

using namespace PlayerCc;

Graphics2dProxy::Graphics2dProxy(PlayerClient *aPc, uint32_t aIndex)
  : ClientProxy(aPc, aIndex),
  mDevice(NULL)
{
  Subscribe(aIndex);
  // how can I get this into the clientproxy.cc?
  // right now, we're dependent on knowing its device type
  mInfo = &(mDevice->info);
}

Graphics2dProxy::~Graphics2dProxy()
{
  Unsubscribe();
}

void
Graphics2dProxy::Subscribe(uint32_t aIndex)
{
  scoped_lock_t lock(mPc->mMutex);
  mDevice = playerc_graphics2d_create(mClient, aIndex);
  if (NULL==mDevice)
    throw PlayerError("Graphics2dProxy::Graphics2dProxy()", "could not create");

  if (0 != playerc_graphics2d_subscribe(mDevice, PLAYER_OPEN_MODE))
    throw PlayerError("Graphics2dProxy::Graphics2dProxy()", "could not subscribe");
}

void
Graphics2dProxy::Unsubscribe()
{
  assert(NULL!=mDevice);
  scoped_lock_t lock(mPc->mMutex);
  playerc_graphics2d_unsubscribe(mDevice);
  playerc_graphics2d_destroy(mDevice);
  mDevice = NULL;
}


void
Graphics2dProxy::Clear( void )
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_graphics2d_clear(mDevice); 
}

void
Graphics2dProxy::DrawPoints( player_point_2d_t pts[], int count )
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_graphics2d_draw_points(mDevice,pts,count); 
}
 
void
Graphics2dProxy::DrawPolyline( player_point_2d_t pts[], int count )
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_graphics2d_draw_polyline(mDevice,pts,count); 
}

void 
Graphics2dProxy::DrawPolygon( player_point_2d_t pts[], int count, bool filled, player_color_t fill_color )
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_graphics2d_draw_polygon(mDevice,pts,count,(int)filled,fill_color); 
}

void
Graphics2dProxy::Color( player_color_t col )
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_graphics2d_setcolor(mDevice, col); 
}

void
Graphics2dProxy::Color( uint8_t red,  uint8_t green,  uint8_t blue,  uint8_t alpha )
{
  player_color_t col;
  col.red = red;
  col.green = green;
  col.blue = blue;
  col.alpha = alpha;

  this->Color( col );
}
