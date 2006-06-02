#ifndef DefendFSM_H
#define DefendFSM_H

#include "GameObj.H"
#include "ScriptObj.H"

#include "FSM.h"
#include "MoveFSM.h"
#include "Vec2d.h"

class DefendFSM : public FSM {
public:
  DefendFSM(GameObj*);
  ~DefendFSM();

  void init(vector<sint4> p);

  int update();
private:
  ScriptObj* weapon;
  Vec2d origin;
  MoveFSM* moveFSM;

  int defendRadius;
  int groundRange;
  int airRange;
};
#endif
