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
 * client-side Health device 
 */

#include "playerc++.h"
#include <cstring>
#include <cstdio>
#include <cmath>
#include <climits>

using namespace PlayerCc;

HealthProxy::HealthProxy(PlayerClient *aPc, uint32_t aIndex)
	: ClientProxy(aPc, aIndex),
  mDevice(NULL)
{
    Subscribe(aIndex);
  // how can I get this into the clientproxy.cc?
  // right now, we're dependent on knowing its device type
    mInfo = &(mDevice->info);
}

HealthProxy::~HealthProxy()
{
    Unsubscribe();
}

void
HealthProxy::Subscribe(uint32_t aIndex)
{
    scoped_lock_t lock(mPc->mMutex);
    mDevice = playerc_health_create(mClient, aIndex);
    if (NULL==mDevice)
	throw PlayerError("HealthProxy::HealthProxy()", "could not create");

    if (0 != playerc_health_subscribe(mDevice, PLAYER_OPEN_MODE))
	throw PlayerError("HealthProxy::HealthProxy()", "could not subscribe");
}

void
HealthProxy::Unsubscribe()
{
    assert(NULL!=mDevice);
    scoped_lock_t lock(mPc->mMutex);
    playerc_health_unsubscribe(mDevice);
    playerc_health_destroy(mDevice);
    mDevice = NULL;
}

float HealthProxy::GetIdleCPU() {
    return mDevice->cpu_usage.idle;
}
float HealthProxy::GetSystemCPU() {
    return mDevice->cpu_usage.system;
}
float HealthProxy::GetUserCPU() {
    return mDevice->cpu_usage.user;
}   
 

int64_t HealthProxy::GetMemTotal() {
    return mDevice->mem.total;
}
    
int64_t HealthProxy::GetMemUsed() {
    return mDevice->mem.used;
}
    
int64_t HealthProxy::GetMemFree() {
    return mDevice->mem.free;
}
    
int64_t HealthProxy::GetSwapTotal() {
    return mDevice->swap.total;
}
 
int64_t HealthProxy::GetSwapUsed() {
    return mDevice->swap.used;
}
    
int64_t HealthProxy::GetSwapFree() {
    return mDevice->swap.free;
}

float HealthProxy::GetPercMemUsed() {	
	return (100.00 * (float)GetMemUsed()/GetMemTotal());
}
    
    
float HealthProxy::GetPercSwapUsed() {
	return (100.00 * (float)(GetSwapUsed())/GetSwapTotal());
}

float HealthProxy::GetPercTotalUsed() {	
	return  (100.00 * (float)(GetMemUsed()+GetSwapUsed())/(GetMemTotal()+GetSwapTotal()));
}
