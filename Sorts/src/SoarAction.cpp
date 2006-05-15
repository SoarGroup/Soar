#include "include/SoarAction.h"
#include <iostream>
#include <string>
using namespace std;

/* This is slow. In the future make some kind of hashtable
 */
ObjectActionType objectActionTypeLookup(string actionName) {
  if      (not actionName.compare("move"))      return OA_MOVE;
  else if (not actionName.compare("mine"))      return OA_MINE;
  else if (not actionName.compare("free"))      return OA_FREE;
  else if (not actionName.compare("attack"))    return OA_ATTACK;
  else                                       return OA_NO_SUCH_ACTION;
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
  else if (not actionName.compare("enable-owner-grouping"))  return AA_OWNER_GROUPING_OFF;
  else                                       return AA_NO_SUCH_ACTION;
}
