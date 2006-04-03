#ifndef soaraction_h
#define soaraction_h
#include <list>

using namespace std;
class SoarGameGroup;

enum SoarActionType {
  SA_IDLE,
  SA_NO_SUCH_ACTION,
  SA_MINE,
  SA_MOVE,
  SA_FREE
};

struct SoarAction {
  SoarActionType type;
  list<SoarGameGroup*> groups;
//  SoarGameGroup* source;
//  SoarGameGroup* target;
  list<int> params;
};

#endif
