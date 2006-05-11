#ifndef AttackFSM_H
#define AttackFSM_H

#include "FSM.h"
#include "MoveFSM.h"
#include "SoarGameObject.h"

class AttackManager;

class AttackFSM : public FSM {

public:
	AttackFSM(GameObj*, AttackManager*);
	~AttackFSM();

	int update();


// these are all commands from the attack manager
  void attack(SoarGameObject* t);
  bool isFiring();
  SoarGameObject* getTarget() { return target; }

  void move(int x, int y);
  bool isMoving() { return moveFSM != NULL; }
  void getDestination(int* x, int* y);
  void stopMoving();

private:
  AttackManager* manager;
  SoarGameObject* target;
  MoveFSM* moveFSM;
  int dest_x, dest_y;
};


#endif
