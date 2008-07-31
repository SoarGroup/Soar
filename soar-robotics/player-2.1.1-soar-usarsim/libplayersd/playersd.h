/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2007
 *     Brian Gerkey
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
/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2007
 *     Brian Gerkey
 *                      
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
 */

/*
 * Interface to libplayersd
 *
 * $Id: playersd.h 4308 2007-12-13 20:23:08Z gerkey $
 */

/** @defgroup libplayersd libplayersd
 * @brief Player service discovery library

This library provides service discovery capabilities for Player devices.  This library is currently in an alpha state, and is not ready for regular use.
*/

/** @ingroup libplayersd
@{ */

#ifndef _PLAYER_SD_H
#define _PLAYER_SD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <libplayercore/player.h>

#define PLAYER_SD_SERVICENAME "_player21._tcp"
#define PLAYER_SD_DEVICE_TXTNAME "device"
#define PLAYER_SD_NAME_MAXLEN 256
#define PLAYER_SD_TXT_MAXLEN 256

/// A device, represented by its name and address
typedef struct player_sd_dev
{
  // Is this entry valid?
  uint8_t valid;
  // Name of the device
  char name[PLAYER_SD_NAME_MAXLEN];
  // Is the address valid?
  uint8_t addr_valid;
  // Did address resolution fail?
  uint8_t addr_fail;
  // Player address components
  char hostname[PLAYER_SD_NAME_MAXLEN];
  uint32_t robot;
  uint16_t interf;
  uint16_t index;
} player_sd_dev_t;

/// Service discovery object
typedef struct player_sd
{
  /// Opaque pointer to underlying zeroconf client object.  Contents
  /// will vary by zeroconf implementation.
  void* sdRef;
  /// List of devices discovered by browsing
  player_sd_dev_t* devs;
  /// Number of devices discovered
  size_t devs_len;
} player_sd_t;

/// Initialize service discovery, passing back a pointer that will be passed
/// into all future calls.  Returns NULL on failure.
player_sd_t* player_sd_init(void);

/// Finalize service discovery, freeing associated resources.
void player_sd_fini(player_sd_t* sd);

/// Lock access to playersd structures, such as the local device cache.
void player_sd_lock(player_sd_t* sd);

/// Unlock access to playersd structures, such as the local device cache.
void player_sd_unlock(player_sd_t* sd);

/// Register the named device.  Returns 0 on success, non-zero on error.
/// Name may be automatically changed in case of conflict.
int player_sd_register(player_sd_t* sd, 
                       const char* name, 
                       player_devaddr_t addr);

/// Unregister (terminate) the named device.  Returns 0 on success, non-zero 
/// on error.
int player_sd_unregister(player_sd_t* sd, 
                         const char* name);

/// Prototype for a callback function that can be invoked when devices are
/// added or removed.
typedef void (*player_sd_browse_callback_fn_t)(player_sd_t* sd,
                                               player_sd_dev_t* dev);

/// Browse for player devices.  Browses for timeout s, accruing the results
/// into the sd object.  If keepalive is non-zero, then the browsing session
/// is left open and can be updated by future calls to player_sd_update().
/// Otherwise, the browsing session is closed before returning.
/// If cb is non-NULL, then it is registered and
/// invoked whenever new device notifications are received (call
/// player_sd_update to give this a chance to happen).  
///
/// NOTE: Because device hostnames are resolved as they are discovered,
/// this call may block for an unpredictable amount of time.
///
/// Returns 0 on success, non-zero on error.
int player_sd_browse(player_sd_t* sd,
                     double timeout, 
                     int keepalive,
                     player_sd_browse_callback_fn_t cb);

/// Check for new device updates, waiting for timeout s.  Contents of sd
/// are updated, and if a callback was passed to player_sd_browse, then this
/// function is also called for each discovered device.  Only makes sense to
/// call this function after a call to player_sd_browse.  
///
/// NOTE: Because device hostnames are resolved as they are discovered,
/// this call may block for an unpredictable amount of time.
///
/// Returns 0 on success, non-zero on error.
int player_sd_update(player_sd_t* sd, double timeout);

/// Stop browsing.  Returns 0 on success, non-zero on error.
int player_sd_browse_stop(player_sd_t* sd);

////////////////////////////////////////////////////////////////////////////
// Common functions, implemented in playersd.c

/// Look up a device by name.  This functions only consults the local cache 
/// of registered devices, which was filled by player_sd_browse().
player_sd_dev_t* player_sd_get_device(player_sd_t* sd, const char* name);

/// Find a device in the local cache that matches the given criteria.
///
///  - sd: a player_sd_t* returned by player_sd_init()
///  - result: a caller-supplied array to store the matching device(s) in
///  - result_len: the length of the result array
///  - name: the Zeroconf name of the device; NULL for don't care
///  - hostname: the hostname where the device resides; NULL for don't care
///  - robot: the 'robot' field of the Player address; -1 for don't care
///  - interf: the 'interf' field of the Player address; -1 for don't care
///  - index: the 'index' field of the Player address; -1 for don't care
///
/// The first result_len devices in the local cache that match the criteria
/// are copied into the result array.
///
/// Returns: the number of devices copied into result.
int player_sd_find_devices(player_sd_t* sd, 
                           player_sd_dev_t* result,
                           size_t result_len,
                           const char* name,
                           const char* hostname,
                           int robot,
                           int interf,
                           int index);

/// Add a device to the local cache.  The user should not call this
/// function.  It will be invoked by the playersd implementation.
player_sd_dev_t* _player_sd_add_device(player_sd_t* sd, const char* name);

/// Print the contents of the local device cache to the console.
void player_sd_printcache(player_sd_t* sd);

#ifdef __cplusplus
}
#endif

/** @} */

#endif
