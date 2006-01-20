#ifndef soaraction_h
#define soaraction_h
#include <vector>

class SoarGameGroup;

enum Action {
  MINE,
  MOVE
};

struct SoarAction {
  Action action;
  vector <SoarGameGroup*> groups;
  vector <int> params;
};

#endif
