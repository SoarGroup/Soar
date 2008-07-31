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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <libplayercore/player.h>  // for interface constants
#include <libplayercore/interface_util.h> // for player_interface_t type

#include "interface_table.c"

static player_interface_t* itable;
static int itable_len;

/*
 * A string table of the message types, used to print type strings in error
 * messages instead of type codes.
 * Must be kept in numerical order with respect to the numeric values of the
 * PLAYER_MSGTYPE_ #defines.
 */
static char* msgTypeStrTable[7]=
{
  "",           // nothing
  "data",       // PLAYER_MSGTYPE_DATA
  "command",    // PLAYER_MSGTYPE_CMD
  "request",    // PLAYER_MSGTYPE_REQ
  "resp_ack",   // PLAYER_MSGTYPE_RESP_ACK
  "sync",       // PLAYER_MSGTYPE_SYNCH
  "resp_nack",  // PLAYER_MSGTYPE_RESP_NACK

};

/*
 * Initialises the interface names/codes table.
 */
int itable_init (void)
{
  int ii;

  // An indirect way of avoiding duplicate initialization.  It would 
  // probably be better to delete the old table, but there may be some
  // pointers hanging around that refer to the old table.
  if(itable)
    return(0);

  for (itable_len = 0; interfaces[itable_len].interf; itable_len++);

  if ((itable = (player_interface_t*) calloc (itable_len, sizeof (player_interface_t))) == NULL)
  {
    printf ("itable_init: Failed to allocate memory for interface table\n");
    return -1;
  }

  for (ii = 0; ii < itable_len; ii++)
  {
    itable[ii].interf = interfaces[ii].interf;
    itable[ii].name = strdup (interfaces[ii].name);
  }

  return 0;
}

/*
 * Grows the interface table to newSize, filling each interface between the
 * old end and the new end with (0xFFFF, "nointerfXX").
 */
int itable_grow (int newSize)
{
  int ii;

  if ((itable = (player_interface_t*) realloc (itable, (newSize * sizeof (player_interface_t)))) == NULL)
  {
    printf("itable_grow: Failed to reallocate table memory\n");
    return -1;
  }
  // Fill in from the old end to the new end with undefined interfaces
  for (ii = itable_len; ii < newSize; ii++)
  {
    itable[ii].interf = 0xFFFF;
    if ((itable[ii].name = (char*) malloc (12)) == NULL)
    {
      printf("itable_grow: Failed to allocate memory for name\n");
      return -1;
    }
    snprintf (itable[ii].name, 12, "nointerf%d", ii);
  }
  // Set the new length
  itable_len = newSize;
  return 0;
}

/*
 * Destroys the interface names/codes table.
 */
void itable_destroy (void)
{
  int ii;

  for (ii = 0; ii < itable_len; ii++)
  {
    if (itable[ii].name != NULL)
      free (itable[ii].name);
  }
  free (itable);
}

/*
 * Add a new interface to the interface table.
 */
int itable_add (const char *name, int code, int replace)
{
  if(code < itable_len)
  {
    // It's already in the table.  Did the caller say to replace?
    if(!replace)
    {
      // Nope; return an error
      return(-1);
    }
    else
    {
      // Yes; replace
      if ((itable[code].name = strdup (name)) == NULL)
      {
        printf("itable_add: Failed to allocate memory for name\n");
        return -1;
      }

      return 0;
    }
  }
  else
  {
    // Not in the table; add it
    if (itable_grow (code + 1) < 0)
    {
      printf ("itable_add: Failed to grow interface table\n");
      return -1;
    }
    itable[code].interf = code;
    if ((itable[code].name = strdup (name)) == NULL)
    {
      printf("itable_add: Failed to allocate memory for name\n");
      return -1;
    }
    return 0;
  }
  return 0;
}

/*
 * looks through the array of available interfaces for one which the given
 * name.  if found, interface is filled out (the caller must provide storage)
 * and zero is returned.  otherwise, -1 is returned.
 */
int
lookup_interface(const char* name, player_interface_t* interface)
{
  int i;
  for(i=0; i<itable_len; i++)
  {
    if(!strcmp(name, itable[i].name))
    {
      *interface = itable[i];
      return 0;
    }
  }
  return -1;
}

/*
 * looks through the array of available interfaces for one which the given
 * code.  if found, interface is filled out (the caller must provide storage)
 * and zero is returned.  otherwise, -1 is returned.
 */
int
lookup_interface_code(int code, player_interface_t* interface)
{
  int i;
  for(i=0; i<itable_len; i++)
  {
    if(code == itable[i].interf)
    {
      *interface = itable[i];
      return 0;
    }
  }
  return -1;
}

/*
 * looks through the array of interfaces, starting at startpos, for the first
 * entry that has the given code, and return the name.
 * leturns 0 when the end of the * array is reached.
 */
const char*
lookup_interface_name(unsigned int startpos, int code)
{
  int i;
  if(startpos > itable_len)
    return 0;
  for(i = startpos; i<itable_len; i++)
  {
    if(code == itable[i].interf)
    {
      return itable[i].name;
    }
  }
  return 0;
}

/*
 * Returns the name of an interface given its code. The result string must
 * not be altered.
 */
const char* interf_to_str(uint16_t code)
{
//   static char unknownstring[15];
  if (code < itable_len)
  {
    if (itable[code].interf != 0xFFFF)
      return itable[code].name;
  }
//   snprintf (unknownstring, 15, "unknown[%d]", code);
  return "unknown";
}

/*
 * Returns the code for an interface, given a string.
 */
uint16_t
str_to_interf(const char *name)
{
  unsigned int ii;
  for(ii=0; ii<itable_len; ii++)
  {
    if(!strcmp(name, itable[ii].name))
      return itable[ii].interf;
  }
  return 0xFFFF;
}

/*
 * Returns the name of a message type given its code. The result string must
 * not be altered.
 */
const char* msgtype_to_str(uint8_t code)
{
//   static char unknownstring[13];
  if (code > 0 && code < 7)
    return msgTypeStrTable[code];
//   snprintf (unknownstring, 15, "unknown[%d]", code);
  return "unknown";
}

/*
 * Returns the code for a message type, given a string.
 */
uint8_t
str_to_msgtype(const char *name)
{
  unsigned int ii;
  for(ii=1; ii < 7; ii++)
  {
    if(!strcmp(name, msgTypeStrTable[ii]))
      return ii;
  }
  return 0xFF;
}
