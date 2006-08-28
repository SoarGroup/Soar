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
#ifndef soaraction_h
#define soaraction_h
#include "general.h"

#include <list>
#include <string>

using namespace std;
class PerceptualGroup;

enum ObjectActionType {
  OA_IDLE,
  OA_NO_SUCH_ACTION,
  OA_MINE,
  OA_MOVE,
  OA_MOVE_MARK,
  OA_MOVE_INTERNAL,
  OA_BUILD,
  OA_ATTACK,
  OA_ATTACK_NEAR,
  OA_ATTACK_MOVE,
  OA_SEVER,
  OA_JOIN,
  OA_FREE,
  OA_TRAIN,
  OA_STICK,
  OA_STOP
};

struct ObjectAction {
  ObjectActionType type;
  list<PerceptualGroup*> groups;
  list<int> params;
};

enum AttentionActionType {
  AA_LOOK_LOCATION,
  AA_MOVE_LOCATION,
  AA_RESIZE,
  AA_LOOK_FEATURE,
  AA_MOVE_FEATURE,
  AA_GROUPING_RADIUS,
  AA_NUM_OBJECTS,
  AA_OWNER_GROUPING_ON,
  AA_OWNER_GROUPING_OFF,
  AA_NO_SUCH_ACTION
};

struct AttentionAction {
    AttentionActionType type;
    list<int> params;
    string fmName;
};

enum GameActionType {
  GA_FIND_BUILDING_LOC,
  GA_SET_MINERAL_BUFFER,
  GA_CLEAR_MINERAL_BUFFER,
  GA_NO_SUCH_ACTION
};

struct GameAction {
  GameActionType type;
  BuildingType building;
  coordinate nearLocation;
  int intValue;
};

ObjectActionType objectActionTypeLookup(string actionName);

AttentionActionType attentionActionTypeLookup(string actionName);
GameActionType gameActionTypeLookup(string actionName);

#endif
