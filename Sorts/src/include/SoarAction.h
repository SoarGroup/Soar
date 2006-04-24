#ifndef soaraction_h
#define soaraction_h
#include <list>
#include <string>

using namespace std;
class SoarGameGroup;

enum ObjectActionType {
  OA_IDLE,
  OA_NO_SUCH_ACTION,
  OA_MINE,
  OA_MOVE,
  OA_FREE
};

struct ObjectAction {
  ObjectActionType type;
  list<SoarGameGroup*> groups;
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
  AA_NO_SUCH_ACTION
};

struct AttentionAction {
    AttentionActionType type;
    list<int> params;
    string fmName;
};
    
ObjectActionType objectActionTypeLookup(string actionName);

AttentionActionType attentionActionTypeLookup(string actionName);

#endif
