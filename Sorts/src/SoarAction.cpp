/*
    This file is part of Sorts, an interface between Soar and ORTS.
    (c) 2006 James Irizarry, Sam Wintermute, and Joseph Xu

    Sorts is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Sorts is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Sorts; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA    
*/
#include "include/SoarAction.h"
#include <iostream>
#include <string>
using namespace std;

/* This is slow. In the future make some kind of hashtable
 */
ObjectActionType objectActionTypeLookup(string actionName) {
  if      (not actionName.compare("move"))     return OA_MOVE;
  else if (not actionName.compare("mine"))     return OA_MINE;
  else if (not actionName.compare("free"))     return OA_FREE;
  else if (not actionName.compare("attack"))   return OA_ATTACK;
  else if (not actionName.compare("sever"))    return OA_SEVER;
  else if (not actionName.compare("join"))    return OA_JOIN;
  else if (not actionName.compare("build"))    return OA_BUILD;
  else if (not actionName.compare("stick"))    return OA_STICK;
  else if (not actionName.compare("train"))    return OA_TRAIN;
  else if (not actionName.compare("stop"))     return OA_STOP;
  else                                         return OA_NO_SUCH_ACTION;
}

AttentionActionType attentionActionTypeLookup(string actionName) {
  if      (not actionName.compare("look-location"))      return AA_LOOK_LOCATION;
  else if (not actionName.compare("look-feature"))      return AA_LOOK_FEATURE;
  else if (not actionName.compare("move-location"))      return AA_MOVE_LOCATION;
  else if (not actionName.compare("move-feature"))      return AA_MOVE_FEATURE;
  else if (not actionName.compare("view-width"))      return AA_RESIZE;
  else if (not actionName.compare("grouping-radius"))  return AA_GROUPING_RADIUS;
  else if (not actionName.compare("num-objects"))  return AA_NUM_OBJECTS;
  else if (not actionName.compare("enable-owner-grouping"))  return AA_OWNER_GROUPING_ON;
  else if (not actionName.compare("disable-owner-grouping"))  return AA_OWNER_GROUPING_OFF;
  else                                       return AA_NO_SUCH_ACTION;
}

GameActionType gameActionTypeLookup(string actionName) {
  if      (not actionName.compare("locate-building")) return GA_FIND_BUILDING_LOC;
  else if (not actionName.compare("increase-mineral-buffer"))  return GA_SET_MINERAL_BUFFER;
  else if (not actionName.compare("clear-mineral-buffer"))  return GA_CLEAR_MINERAL_BUFFER;
  else                                       return GA_NO_SUCH_ACTION;
}
