#ifndef AttackFSM_H
#define AttackFSM_H

#include "Point.h"
#include "FSM.h"
#include "MoveFSM.h"
#include "SoarGameObject.h"

class AttackManager;

class AttackFSM : public FSM {

public:
	AttackFSM(GameObj*);
	~AttackFSM();

  void init(vector<sint4> p);
	int update();

// these are all commands from the attack manager
  void attack(SoarGameObject* t);
  bool isFiring();
  SoarGameObject* getTarget() { return target; }

  void move(int x, int y);
  bool isMoving() { return moving; }
  Point getDestination() { return dest; }
  void stopMoving();

  // AttackManager is about to be deallocated, 
  void disown(int lastStatus);

private:
  AttackManager* manager;

  SoarGameObject* target;
  ScriptObj* weapon;

  MoveFSM* moveFSM;
  Point dest;

public: // for debug only
  bool moving;

  int disownedStatus;
};


#endif
