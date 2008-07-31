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
 * $Id: cameraproxy.cc 4227 2007-10-24 22:32:04Z thjc $
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

CameraProxy::CameraProxy(PlayerClient *aPc, uint32_t aIndex)
  : ClientProxy(aPc, aIndex),
  mDevice(NULL),
  mPrefix("image"),
  mFrameNo(0)
{
  Subscribe(aIndex);
  // how can I get this into the clientproxy.cc?
  // right now, we're dependent on knowing its device type
  mInfo = &(mDevice->info);
}

CameraProxy::~CameraProxy()
{
  Unsubscribe();
}

void
CameraProxy::Subscribe(uint32_t aIndex)
{
  scoped_lock_t lock(mPc->mMutex);
  mDevice = playerc_camera_create(mClient, aIndex);
  if (NULL==mDevice)
    throw PlayerError("CameraProxy::CameraProxy()", "could not create");

  if (0 != playerc_camera_subscribe(mDevice, PLAYER_OPEN_MODE))
    throw PlayerError("CameraProxy::CameraProxy()", "could not subscribe");
}

void
CameraProxy::Unsubscribe()
{
  assert(NULL!=mDevice);
  scoped_lock_t lock(mPc->mMutex);
  playerc_camera_unsubscribe(mDevice);
  playerc_camera_destroy(mDevice);
  mDevice = NULL;
}

void
CameraProxy::SaveFrame(const std::string aPrefix, uint32_t aWidth)
{
  std::ostringstream filename;
  filename.imbue(std::locale(""));
  filename.fill('0');

  filename << aPrefix << std::setw(aWidth) << mFrameNo++;
  if (GetCompression())
    filename << ".jpg";
  else
    filename << ".ppm";

  scoped_lock_t lock(mPc->mMutex);
  playerc_camera_save(mDevice, filename.str().c_str());
}

void
CameraProxy::Decompress()
{
  scoped_lock_t lock(mPc->mMutex);
  playerc_camera_decompress(mDevice);
}

std::ostream& std::operator << (std::ostream& os, const PlayerCc::CameraProxy& c)
{
  return os << c.GetWidth() << "\t"
            << c.GetHeight() << "\t"
            << 1/c.GetElapsedTime() << "\t"
            << c.GetDataTime() << "\t"
            << (c.GetCompression() ? "compressed" : "");
}
