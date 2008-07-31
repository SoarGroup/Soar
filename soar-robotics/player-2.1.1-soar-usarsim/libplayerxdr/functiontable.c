/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2005 -
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
 * $Id: functiontable.c 4162 2007-09-21 03:31:51Z thjc $
 *
 * Functions for looking up the appropriate XDR pack/unpack function for a
 * given message type and subtype.
 */

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

// Can't use libplayererror here because of an unresolved circular build
// dependency
//#include <libplayercore/error.h>

#include "playerxdr.h"
#include "functiontable.h"

static playerxdr_function_t init_ftable[] =
{
  /* This list is currently alphabetized, please keep it that way! */
  /* universal messages */
  {0, PLAYER_MSGTYPE_REQ, PLAYER_CAPABILTIES_REQ,
   (player_pack_fn_t)player_capabilities_req_pack, NULL, NULL},
  {0, PLAYER_MSGTYPE_REQ, PLAYER_GET_INTPROP_REQ,
   (player_pack_fn_t)player_intprop_req_pack, (player_copy_fn_t)player_intprop_req_t_copy, (player_cleanup_fn_t)player_intprop_req_t_cleanup, 
   (player_clone_fn_t)player_intprop_req_t_clone,(player_free_fn_t)player_intprop_req_t_free,(player_sizeof_fn_t)player_intprop_req_t_sizeof},
  {0, PLAYER_MSGTYPE_REQ, PLAYER_SET_INTPROP_REQ,
   (player_pack_fn_t)player_intprop_req_pack, (player_copy_fn_t)player_intprop_req_t_copy, (player_cleanup_fn_t)player_intprop_req_t_cleanup,
   (player_clone_fn_t)player_intprop_req_t_clone,(player_free_fn_t)player_intprop_req_t_free,(player_sizeof_fn_t)player_intprop_req_t_sizeof},
  {0, PLAYER_MSGTYPE_REQ, PLAYER_GET_DBLPROP_REQ,
   (player_pack_fn_t)player_dblprop_req_pack, (player_copy_fn_t)player_dblprop_req_t_copy, (player_cleanup_fn_t)player_dblprop_req_t_cleanup,
   (player_clone_fn_t)player_dblprop_req_t_clone,(player_free_fn_t)player_dblprop_req_t_free,(player_sizeof_fn_t)player_dblprop_req_t_sizeof},
  {0, PLAYER_MSGTYPE_REQ, PLAYER_SET_DBLPROP_REQ,
   (player_pack_fn_t)player_dblprop_req_pack, (player_copy_fn_t)player_dblprop_req_t_copy, (player_cleanup_fn_t)player_dblprop_req_t_cleanup,
   (player_clone_fn_t)player_dblprop_req_t_clone,(player_free_fn_t)player_dblprop_req_t_free,(player_sizeof_fn_t)player_dblprop_req_t_sizeof},
  {0, PLAYER_MSGTYPE_REQ, PLAYER_GET_STRPROP_REQ,
   (player_pack_fn_t)player_strprop_req_pack, (player_copy_fn_t)player_strprop_req_t_copy, (player_cleanup_fn_t)player_strprop_req_t_cleanup,
   (player_clone_fn_t)player_strprop_req_t_clone,(player_free_fn_t)player_strprop_req_t_free,(player_sizeof_fn_t)player_strprop_req_t_sizeof},
  {0, PLAYER_MSGTYPE_REQ, PLAYER_SET_STRPROP_REQ,
   (player_pack_fn_t)player_strprop_req_pack, (player_copy_fn_t)player_strprop_req_t_copy, (player_cleanup_fn_t)player_strprop_req_t_cleanup,
   (player_clone_fn_t)player_strprop_req_t_clone,(player_free_fn_t)player_strprop_req_t_free,(player_sizeof_fn_t)player_strprop_req_t_sizeof},

  /* Special messages */
  {PLAYER_PLAYER_CODE, PLAYER_MSGTYPE_SYNCH, 0,
    (player_pack_fn_t)player_add_replace_rule_req_pack, NULL, NULL, NULL, NULL, NULL},

  /* generated messages from the interface definitions */
#include "functiontable_gen.c"

  /* This NULL element signals the end of the list; don't remove it */
  {0,0,0,NULL,NULL,NULL}
};

static playerxdr_function_t* ftable;
static int ftable_len;

void
playerxdr_ftable_init()
{
  playerxdr_function_t* f;
  ftable_len = 0;
  for(f = init_ftable; f->packfunc; f++)
    ftable_len++;

  ftable = (playerxdr_function_t*)calloc(ftable_len,
                                         sizeof(playerxdr_function_t));
  assert(ftable);

  memcpy(ftable,init_ftable,ftable_len*sizeof(playerxdr_function_t));
}

int
playerxdr_ftable_add(playerxdr_function_t f, int replace)
{
  if(playerxdr_get_packfunc(f.interf, f.type, f.subtype))
  {
    // It's already in the table.  Did the caller say to replace?
    if(!replace)
    {
      // Nope; return an error
      return(-1);
    }
    else
    {
      // Yes; replace (it's clearly inefficient to iterate through the
      // table again to find the entry to replace, but the table is pretty
      // small and this doesn't happen very often)
      int i;
      playerxdr_function_t* curr;

      for(i=0;i<ftable_len;i++)
      {
        curr = ftable + i;
        // Make sure the interface, type, and subtype match exactly
        if((curr->interf == f.interf) &&
           (curr->subtype == f.subtype) &&
           (curr->type == f.type))
        {
          *curr = f;
          return(0);
        }
      }
      // Can't use libplayererror here because of an unresolved circular build
      // dependency
      //PLAYER_ERROR("unable to find entry to replace");
      puts("playerxdr_ftable_add: unable to find entry to replace");
      return(-1);
    }
  }
  else
  {
    // Not in the table; add it
    ftable = (playerxdr_function_t*)realloc(ftable,
                                            ((ftable_len+1)*
                                             sizeof(playerxdr_function_t)));
    assert(ftable);
    ftable[ftable_len++] = f;
    return(0);
  }
}

int
playerxdr_ftable_add_multi(playerxdr_function_t *flist, int replace)
{
  playerxdr_function_t* f;

  for (f = flist; f->packfunc; f++)
  {
    if (playerxdr_ftable_add (*f, replace) < 0)
    {
      puts("Failed to add new function to XDR function table");
      return(-1);
    }
  }
  return(0);
}

playerxdr_function_t*
playerxdr_get_ftrow(uint16_t interf, uint8_t type, uint8_t subtype)
{
  playerxdr_function_t* curr=NULL;
  int i;

  if(!ftable_len)
    return(NULL);

  for(i=0;i<ftable_len;i++)
  {
    curr = ftable + i;
    // Make sure the interface and subtype match exactly.
    // match anyway if interface = 0 (universal data types)

    if((curr->interf == interf || curr->interf == 0) &&
      curr->type == type &&
      curr->subtype == subtype)
      return(curr);
  }

  // The supplied type can be RESP_ACK if the registered type is REQ.
  if (type == PLAYER_MSGTYPE_RESP_ACK || type == PLAYER_MSGTYPE_RESP_NACK)
    type = PLAYER_MSGTYPE_REQ;

  for(i=0;i<ftable_len;i++)
  {
    curr = ftable + i;
    // Make sure the interface and subtype match exactly.
    // match anyway if interface = 0 (universal data types)
    if((curr->interf == interf || curr->interf == 0) &&
      curr->type == type &&
      curr->subtype == subtype)
      return(curr);
  }

  return(NULL);
}

player_pack_fn_t
playerxdr_get_packfunc(uint16_t interf, uint8_t type, uint8_t subtype)
{
  playerxdr_function_t* row=NULL;

  if ((row = playerxdr_get_ftrow (interf, type, subtype)) != NULL)
    return(row->packfunc);

  return(NULL);
}

player_copy_fn_t
playerxdr_get_copyfunc(uint16_t interf, uint8_t type, uint8_t subtype)
{
  playerxdr_function_t* row=NULL;

  if ((row = playerxdr_get_ftrow (interf, type, subtype)) != NULL)
    return(row->copyfunc);

  return(NULL);
}

player_cleanup_fn_t
playerxdr_get_cleanupfunc(uint16_t interf, uint8_t type, uint8_t subtype)
{
  playerxdr_function_t* row=NULL;

  if ((row = playerxdr_get_ftrow (interf, type, subtype)) != NULL)
    return(row->cleanupfunc);

  return(NULL);
}

player_clone_fn_t
playerxdr_get_clonefunc(uint16_t interf, uint8_t type, uint8_t subtype)
{
  playerxdr_function_t* row=NULL;

  if ((row = playerxdr_get_ftrow (interf, type, subtype)) != NULL)
    return(row->clonefunc);

  return(NULL);
}

player_free_fn_t
playerxdr_get_freefunc(uint16_t interf, uint8_t type, uint8_t subtype)
{
  playerxdr_function_t* row=NULL;

  if ((row = playerxdr_get_ftrow (interf, type, subtype)) != NULL)
    return(row->freefunc);

  return(NULL);
}

player_sizeof_fn_t
playerxdr_get_sizeoffunc(uint16_t interf, uint8_t type, uint8_t subtype)
{
  playerxdr_function_t* row=NULL;

  if ((row = playerxdr_get_ftrow (interf, type, subtype)) != NULL)
    return(row->sizeoffunc);

  return(NULL);
}

// Deep copy a message structure
unsigned int
playerxdr_deepcopy_message(void* src, void* dest, uint16_t interf, uint8_t type, uint8_t subtype)
{
  player_copy_fn_t copyfunc = NULL;

  if ((copyfunc = playerxdr_get_copyfunc(interf, type, subtype)) == NULL)
    return 0;

  return (*copyfunc)(dest, src);
}

void *
playerxdr_clone_message(void* msg, uint16_t interf, uint8_t type, uint8_t subtype)
{
  player_clone_fn_t clonefunc = NULL;

  if ((clonefunc = playerxdr_get_clonefunc(interf, type, subtype)) == NULL)
    return NULL;

  return (*clonefunc)(msg);
}


void
playerxdr_free_message(void* msg, uint16_t interf, uint8_t type, uint8_t subtype)
{
  player_free_fn_t freefunc = NULL;

  if ((freefunc = playerxdr_get_freefunc(interf, type, subtype)) == NULL)
    return;

  (*freefunc)(msg);
}
void
playerxdr_cleanup_message(void* msg, uint16_t interf, uint8_t type, uint8_t subtype)
{
  player_cleanup_fn_t cleanupfunc = NULL;

  if ((cleanupfunc = playerxdr_get_cleanupfunc(interf, type, subtype)) == NULL)
    return;

  (*cleanupfunc)(msg);
}
