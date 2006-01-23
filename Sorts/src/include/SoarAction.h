#ifndef soaraction_h
#define soaraction_h
#include <list>

class SoarGameGroup;

enum SoarActionType {
  SA_NO_SUCH_ACTION,
  SA_MINE,
  SA_MOVE
};

struct SoarAction {
  SoarActionType type;
  list<SoarGameGroup*> groups;
  list<int> params;
};

#endif
