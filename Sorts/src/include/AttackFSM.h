#ifndef AttackFSM_H
#define AttackFSM_H

#include "Vec2d.h"
#include "FSM.h"
#include "MoveFSM.h"
#include "SoarGameObject.h"

class AttackManager;

class AttackFSM : public FSM {

public:
	AttackFSM(SoarGameObject* sgob);
	~AttackFSM();

  void init(vector<sint4> p);
	int update();

// these are all commands from the attack manager
  void attack(SoarGameObject* t);
  bool isFiring();

  int move(int x, int y);
  bool isMoving() { return moving; }
  Vec2d getDestination() { return dest; }
  void stopMoving();

  // AttackManager is about to be deallocated, 
  void disown(int lastStatus);

  void stop();

  int getAvgDamage() {
    return (weapon->get_int("min_damage") + weapon->get_int("max_damage")) / 2;
  }

public:
  bool reassign;
  int failCount;
  SoarGameObject* target;
  bool waitingForCatchup;
  bool panic;

private:
  SoarGameObject* sgob;
  AttackManager* manager;
  ScriptObj* weapon;
  MoveFSM* moveFSM;
  Vec2d dest;
  Vector<sint4> attackParams;
  bool moving;
  int disownedStatus;
  
  bool firstMove;
  int moveFails;
};


#endif
