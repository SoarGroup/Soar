/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000
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
 * $Id: drivertable.h 6499 2008-06-10 01:13:51Z thjc $
 *
 *   class to keep track of available drivers.
 */

#ifndef _DRIVERTABLE_H
#define _DRIVERTABLE_H

#include <pthread.h>

#include <libplayercore/device.h>
#include <libplayercore/configfile.h>

// Forward declarations
class DriverTable;
class ConfigFile;

/// @brief Function signature for driver factory functions
typedef Driver* (*DriverInitFn) (ConfigFile *cf, int section);

/// @brief Function signature for driver plugin initialization functions
typedef int (*DriverPluginInitFn) (DriverTable* table);


/// @brief Info about an individual driver class
class DriverEntry
{
  public:

  // factory creation function
  DriverInitFn initfunc;

  // the string name for the driver
  char name[PLAYER_MAX_DRIVER_STRING_LEN];

  // next in list
  DriverEntry* next;

  DriverEntry() { name[0]='\0'; next = NULL; }
};


/// @brief List of available driver classes.
///
/// This class maintains a list of driver names and factory functions;
/// it is used to instantiate drivers at run-time.
class DriverTable
{
  private:

  // we'll keep the driver info here.
  DriverEntry* head;
  int numdrivers;

  public:

  DriverTable();
  ~DriverTable();

  /// @brief Add a driver class to the table
  /// @param name Driver name (as it appears in the configuration file).
  /// @param initfunc Driver factory function.
  int AddDriver(const char* name, DriverInitFn initfunc);

  /// @brief Lookup a driver entry by name
  /// @param name Driver name.
  DriverEntry* GetDriverEntry(const char* name);

  /// @brief Get the number of registered driver classes.
  int Size() { return(numdrivers); }

  /// @brief Lookup a driver name by index.
  /// @param idx Index in list.
  /// @returns Returns NULL if there is no matching driver.
  char* GetDriverName(int idx);

  /// @brief Sort drivers, based on name.
  /// @returns Returns a pointer to newly malloc()ed memory, which the
  /// user should free().
  char** SortDrivers();
};

#endif
