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

// TODO: change to GameAction
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
