#include "DefendFSM.h"

DefendFSM::DefendFSM(GameObj* gob) : FSM(gob) { 
  weapon = gob->component("weapon");
  assert (weapon != NULL);

  groundRange = weapon->get_int("max_ground_range");
  airRange = weapon->get_int("max_air_range");

  moveFSM = NULL;
}

DefendFSM::~DefendFSM() { 
  if (moveFSM != NULL) {
    delete moveFSM;
  }
}

void DefendFSM::init(vector<sint4> p) {
  origin = Vec2d(p[0], p[1]);
  defendRadius = p[2];
}

int DefendFSM::update() {
  list<GameObj*> collisions;
  Sorts::spatialDB->getCollisions(origin(0), origin(1), defendRadius, NULL, collisions);

  GameObj* biggestThreat = NULL;
  double threatRatio = 0; // avg damage rate / hp
  for(list<GameObj*>::iterator
      i  = collisions.begin();
      i != collisions.end();
      i++)
  {
    if (*(*i)->sod.owner != Sorts::OrtsIO->getMyId()) {
      double dmgRate = weaponDamageRate(*i);
      int hp = (*i)->get_int("hp");
      double ratio = dmgRate / hp;
      if (ratio > threatRatio) {
        biggestThreat = *i;
        threatRatio = ratio;
      }
    }
  }
  if (biggestThreat == NULL) {
    return FSM_RUNNING;
  }

  if (canHit(gob, biggestThreat)) {
    Vector<sint4> attackParams(1);
    attackParams[0] = Sorts::OrtsIO->getGobId(gob);
    weapon->set_action("attack", attackParams);
  }
  else {
    if (moveFSM == NULL) {
      moveFSM = new MoveFSM(gob);
    }
    Vector<sint4> moveParams(3);
  }
}
