#ifndef general_h
#define general_h

class SoarGameGroup;

enum Action {
  MINE,
  MOVE
};

struct SoarAction {
  Action action;
  SoarGameGroup* group;
};

#endif
