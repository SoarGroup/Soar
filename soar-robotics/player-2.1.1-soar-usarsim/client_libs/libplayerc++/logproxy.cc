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
 * $Id: logproxy.cc 6307 2008-04-13 04:40:06Z thjc $
 */

#include "playerc++.h"

using namespace PlayerCc;

LogProxy::LogProxy(PlayerClient *aPc, uint32_t aIndex)
  : ClientProxy(aPc, aIndex),
  mDevice(NULL)
{
  Subscribe(aIndex);
  // how can I get this into the clientproxy.cc?
  // right now, we're dependent on knowing its device type
  mInfo = &(mDevice->info);
}

LogProxy::~LogProxy()
{
  Unsubscribe();
}

void
LogProxy::Subscribe(uint32_t aIndex)
{
  scoped_lock_t lock(mPc->mMutex);
  mDevice = playerc_log_create(mClient, aIndex);
  if (NULL==mDevice)
    throw PlayerError("LogProxy::LogProxy()", "could not create");

  if (0 != playerc_log_subscribe(mDevice, PLAYER_OPEN_MODE))
    throw PlayerError("LogProxy::LogProxy()", "could not subscribe");
}

void
LogProxy::Unsubscribe()
{
  assert(NULL!=mDevice);
  scoped_lock_t lock(mPc->mMutex);
  playerc_log_unsubscribe(mDevice);
  playerc_log_destroy(mDevice);
  mDevice = NULL;
}

void LogProxy::QueryState()
{
  scoped_lock_t lock(mPc->mMutex);

  if (0 != playerc_log_get_state(mDevice))
    throw PlayerError("LogProxy::QueryState()", "error querying state");
  return;
}

void
LogProxy::SetState(int aState)
{
  scoped_lock_t lock(mPc->mMutex);

  if (mDevice->type == 0) {
    if (0 != playerc_log_get_state(mDevice))
      throw PlayerError("LogProxy::SetState()", "error querying type");
  }

  if (mDevice->type == PLAYER_LOG_TYPE_READ) {
    if (0 != playerc_log_set_read_state(mDevice,aState))
      throw PlayerError("LogProxy::SetState()", "error setting read");
  } else if(mDevice->type == PLAYER_LOG_TYPE_WRITE) {
    if (0 != playerc_log_set_write_state(mDevice,aState))
      throw PlayerError("LogProxy::SetState()", "error setting write");
  } else {
    // unknown type
  }
  return;
}

void
LogProxy::SetWriteState(int aState)
{
  scoped_lock_t lock(mPc->mMutex);
  if (0 != playerc_log_set_write_state(mDevice,aState))
    throw PlayerError("LogProxy::SetWriteState()", "error setting write");
  return;
}

void
LogProxy::SetReadState(int aState)
{
  scoped_lock_t lock(mPc->mMutex);
  if (0 != playerc_log_set_read_state(mDevice,aState))
    throw PlayerError("LogProxy::SetReadState()", "error setting read");
  return;
}

void
LogProxy::Rewind()
{
  scoped_lock_t lock(mPc->mMutex);
  if (0 != playerc_log_set_read_rewind(mDevice))
    throw PlayerError("LogProxy::Rewind()", "error rewinding");
  return;
}

void
LogProxy::SetFilename(const std::string aFilename)
{
  scoped_lock_t lock(mPc->mMutex);
  if (0 != playerc_log_set_filename(mDevice,aFilename.c_str()))
    throw PlayerError("LogProxy::SetFilename()", "error setting filename");
  return;
}

std::ostream&
std::operator << (std::ostream &os, const PlayerCc::LogProxy &c)
{
  os << "#Log (" << c.GetInterface() << ":" << c.GetIndex() << ")" << std::endl;

  return os;
}
