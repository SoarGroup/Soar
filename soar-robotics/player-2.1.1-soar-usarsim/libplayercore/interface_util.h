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

/** @defgroup libplayerutils libplayerutils
 * @brief Player utility library

This library provides miscellaneous utilities functions
*/

/** @ingroup libplayerutils
@{ */

#ifndef _INTERFACE_UTIL_H
#define _INTERFACE_UTIL_H

#include <libplayercore/playerconfig.h>  // for uint16_t type

#ifdef __cplusplus
extern "C" {
#endif

// available interfaces are stored in an array of these, defined in
// interface_util.c
typedef struct
{
  uint16_t interf;
  char* name;
} player_interface_t;

/*
 * Initialises the interface names/codes table.
 */
int itable_init (void);

/*
 * Grows the interface table to newSize, filling each interface between the
 * old end and the new end with (0xFFFF, "nointerfXX").
 */
int itable_grow (int newSize);

/*
 * Destroys the interface names/codes table.
 */
void itable_destroy (void);

/*
 * Add a new interface to the interface table.
 */
int itable_add (const char *name, int code, int replace);

/*
 * looks through the array of available interfaces for one which the given
 * name.  if found, interface is filled out (the caller must provide storage)
 * and zero is returned.  otherwise, -1 is returned.
 */
int lookup_interface(const char* name, player_interface_t* interface);

/*
 * looks through the array of available interfaces for one which the given
 * code.  if found, interface is filled out (the caller must provide storage)
 * and zero is returned.  otherwise, -1 is returned.
 */
int
lookup_interface_code(int code, player_interface_t* interface);

/*
 * looks through the array of interfaces, starting at startpos, for the first
 * entry that has the given code, and returns the name.
 * returns 0 if the device is not found.
 */
const char*
lookup_interface_name(unsigned int startpos, int code);

/*
 * Returns the name of an interface given its code. The result string must
 * not be altered.
 */
const char*
interf_to_str(uint16_t code);

/*
 * Returns the code for an interface, given a string. If the name is not found,
 * 0xFFFF is returned.
 */
uint16_t
str_to_interf(const char *name);

/*
 * Returns the name of a message type given its code. The result string must
 * not be altered.
 */
const char*
msgtype_to_str(uint8_t code);

/*
 * Returns the code for a message type, given a string. If the name is not
 * found, 0xFF is returned.
 */
uint8_t
str_to_msgtype(const char *name);

#ifdef __cplusplus
}
#endif

/** @} */

#endif
