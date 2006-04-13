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
  else                                       return OA_NO_SUCH_ACTION;
}

AttentionActionType attentionActionTypeLookup(string actionName) {
  if      (not actionName.compare("look-location"))      return AA_LOOK_LOCATION;
  else if (not actionName.compare("look-feature"))      return AA_LOOK_FEATURE;
  else if (not actionName.compare("move-location"))      return AA_MOVE_LOCATION;
  else if (not actionName.compare("move-feature"))      return AA_MOVE_FEATURE;
  else if (not actionName.compare("resize"))      return AA_RESIZE;
  else if (not actionName.compare("grouping-radius"))  return AA_GROUPING_RADIUS;
  else                                       return AA_NO_SUCH_ACTION;
}
