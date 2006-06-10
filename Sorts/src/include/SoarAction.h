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
  OA_MOVE_INTERNAL,
  OA_BUILD,
  OA_ATTACK,
  OA_ATTACK_NEAR,
  OA_ATTACK_MOVE,
  OA_SEVER,
  OA_FREE,
  OA_TRAIN,
  OA_STICK
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

enum MapActionType {
  MA_FIND_BUILDING_LOC,
  MA_NO_SUCH_ACTION
};

struct MapAction {
  MapActionType type;
  BuildingType building;
  coordinate nearLocation;
  int minDistance;
};

ObjectActionType objectActionTypeLookup(string actionName);

AttentionActionType attentionActionTypeLookup(string actionName);
MapActionType mapActionTypeLookup(string actionName);

#endif
