#include <sys/time.h>

#include "AttackManager.h"
#include "general.h"
#include "AttackManagerRegistry.h"
#include "Circle.h"
#include "Sorts.h"

#include "ScriptObj.H"

#define msg cout << "ATKMAN(" << (int)this << "): "

#define WAIT_RATIO    0.5
#define RESUME_RATIO  0.85

#ifdef USE_CANVAS
#define USE_CANVAS_ATTACK_MANAGER
#endif

#define NO_WAITING
  
inline int min(int a, int b) {
  if (a < b) {
    return a;
  }
  return b;
}

inline int max(int a, int b) {
  if (a > b) {
    return a;
  }
  return b;
}

inline int minRadius(GameObj* gob) {
  switch (*gob->sod.shape) {
    case SHAPE_RECTANGLE:
      return min((*gob->sod.x2-*gob->sod.x1), (*gob->sod.y2-*gob->sod.y1))/2;
    case SHAPE_CIRCLE:
      return *gob->sod.radius;
    default:
      assert(false);
  }
}

inline int maxRadius(GameObj* gob) {
  switch (*gob->sod.shape) {
    case SHAPE_RECTANGLE:
      return max((*gob->sod.x2-*gob->sod.x1), (*gob->sod.y2-*gob->sod.y1))/2;
    case SHAPE_CIRCLE:
      return *gob->sod.radius;
    default:
      assert(false);
      return *gob->sod.radius;
  }
}

Vec2d getClosePos(GameObj* src, GameObj* tgt) {
  int srcRadius = maxRadius(src);
  int tgtRadius = maxRadius(tgt);
  Vec2d srcPos(gobX(src), gobY(src));
  Vec2d tgtPos(gobX(tgt), gobY(tgt));

  return tgtPos - Vec2d(tgtPos - srcPos, srcRadius + tgtRadius);
}


void positionsOnRectangle
( int ax, int ay, int tx1, int ty1, int tx2, int ty2, 
  int offset, int distBetween, 
  list<Vec2d>& positions)
{
  enum Side { LEFT, RIGHT, TOP, BOTTOM };
  Side s[4];

  int posX1 = tx1 - offset;
  int posY1 = ty1 - offset;
  int posX2 = tx2 + offset;
  int posY2 = ty2 + offset;

  int closeX = -1, closeY = -1;

  if (ax < tx1) {
    if (ay < ty1) {
      // upper left corner
      int diffx = tx1 - ax;
      int diffy = ty1 - ay;
      if (diffx < diffy) {
        s[0] = TOP; s[1] = LEFT; s[2] = BOTTOM; s[3] = RIGHT;
        closeX = tx1;
      }
      else {
        s[0] = LEFT; s[1] = TOP; s[2] = RIGHT; s[3] = BOTTOM;
        closeY = ty1;
      }
    }
    else if (ay > ty2) {
      // lower left corner
      int diffx = tx1 - ax;
      int diffy = ay - ty2;
      if (diffx < diffy) {
        s[0] = BOTTOM; s[1] = LEFT; s[2] = TOP; s[3] = RIGHT;
        closeX = tx1;
      }
      else {
        s[0] = LEFT; s[1] = BOTTOM; s[2] = RIGHT; s[3] = TOP;
        closeY = ty2;
      }
    }
    else {
      // left side
      s[0] = LEFT; s[1] = BOTTOM; s[2] = TOP; s[3] = RIGHT;
      closeY = ay;
    }
  }
  else if (ax > tx2) {
    if (ay < ty1) {
      // upper right corner
      int diffx = ax - tx2;
      int diffy = ty1 - ay;
      if (diffx < diffy) {
        s[0] = TOP; s[1] = RIGHT; s[2] = BOTTOM; s[3] = LEFT;
        closeX = tx2;
      }
      else {
        s[0] = RIGHT; s[1] = TOP; s[2] = LEFT; s[3] = BOTTOM;
        closeY = ty1;
      }
    }
    else if (ay > ty2) {
      // lower right corner
      int diffx = ax - tx2;
      int diffy = ay - ty2;
      if (diffx < diffy) {
        s[0] = BOTTOM; s[1] = RIGHT; s[2] = TOP; s[3] = LEFT;
        closeX = tx2;
      }
      else {
        s[0] = RIGHT; s[1] = BOTTOM; s[2] = LEFT; s[3] = TOP;
        closeY = ty2;
      }
    }
    else {
      // right side
      s[0] = RIGHT; s[1] = TOP; s[2] = BOTTOM; s[3] = LEFT;
      closeY = ay;
    }
  }
  else if (ay < ty1) {
    // top
    s[0] = TOP; s[1] = RIGHT; s[2] = LEFT; s[3] = BOTTOM;
    closeX = ax;
  }
  else {
    // bottom
    s[0] = BOTTOM; s[1] = LEFT; s[2] = RIGHT; s[3] = TOP;
    closeX = ax;
  }
 
//  msg << "PATTERN START" << endl;
  int d;
  for(int i = 0; i < 4; i++) {
    switch(s[i]) {
      case TOP:
        assert(tx1 <= closeX && closeX <= tx2);
        positions.push_back(Vec2d(closeX, posY1));
        for(d=distBetween; closeX-d>=tx1 || closeX+d<=tx2; d+=distBetween) {
          if (closeX-d >= tx1) {
            positions.push_back(Vec2d(closeX-d, posY1));
//            msg << "PATTERN: " << closeX-d << " " << posY1 << endl;
          }
          if (closeX+d <= tx2) {
            positions.push_back(Vec2d(closeX+d, posY1));
//            msg << "PATTERN: " << closeX+d << " " << posY1 << endl;
          }
        }
        closeY = ty1; // for next side
        break;
      case BOTTOM:
        assert(tx1 <= closeX && closeX <= tx2);
        positions.push_back(Vec2d(closeX, posY2));
        for(d=distBetween; closeX-d>=tx1 || closeX+d<=tx2; d+=distBetween) {
          if (closeX-d >= tx1) {
            positions.push_back(Vec2d(closeX-d, posY2));
//            msg << "PATTERN: " << closeX-d << " " << posY2 << endl;
          }
          if (closeX+d <= tx2) {
            positions.push_back(Vec2d(closeX+d, posY2));
//            msg << "PATTERN: " << closeX+d << " " << posY2 << endl;
          }
        }
        closeY = ty2;
        break;
      case LEFT:
        assert(ty1 <= closeY && closeY <= ty2);
        positions.push_back(Vec2d(posX1, closeY));
        for(d=distBetween; closeY-d>=ty1 || closeY+d<=ty2; d+=distBetween) {
          if (closeY-d >= ty1) {
            positions.push_back(Vec2d(posX1, closeY-d));
//            msg << "PATTERN: " << posX1 << " " << closeY-d << endl;
          }
          if (closeY+d <= ty2) {
            positions.push_back(Vec2d(posX1, closeY+d));
//            msg << "PATTERN: " << posX1 << " " << closeY+d << endl;
          }
        }
        closeX = tx1;
        break;
      case RIGHT:
        assert(ty1 <= closeY && closeY <= ty2);
        positions.push_back(Vec2d(posX2, closeY));
        for(d=distBetween; closeY-d>=ty1 || closeY+d<=ty2; d+=distBetween) {
          if (closeY-d >= ty1) {
            positions.push_back(Vec2d(posX2, closeY-d));
//            msg << "PATTERN: " << posX2 << " " << closeY-d << endl;
          }
          if (closeY+d <= ty2) {
            positions.push_back(Vec2d(posX2, closeY+d));
//            msg << "PATTERN: " << posX2 << " " << closeY+d << endl;
          }
        }
        closeX = tx2;
        break;
      default:
        assert(false);
    }
  }
//  msg << "PATTERN END" << endl;
}

void AttackManager::attackArcPos
( GameObj* atk, 
  GameObj* tgt, 
  int layer,
  list<Vec2d>& positions) 
{
  int atkRadius = *atk->sod.radius;
  Vec2d aPos(*atk->sod.x, *atk->sod.y);
  Vec2d tPos(*tgt->sod.x, *tgt->sod.y);

  int range;
  if (*tgt->sod.zcat == GameObj::ON_LAND) {
    range = atk->component("weapon")->get_int("max_ground_range") 
      + atkRadius;
  }
  else {
    range = atk->component("weapon")->get_int("max_air_range") 
      + atkRadius;
  }

  range = (int)(range * 0.8); // for safety
  range -= layer * atkRadius * 2;

  list<Vec2d> atkPos;
  if (*tgt->sod.shape == SHAPE_RECTANGLE) {
    int halfwidth = (*tgt->sod.x2 - *tgt->sod.x1) /  2;
    int halfheight = (*tgt->sod.y2 - *tgt->sod.y1) /  2;

    int minRadius = min(halfwidth, halfheight);
    int maxRadius = max(halfwidth, halfheight);

    msg << "USING RADIUS " << minRadius << endl;

    if (maxRadius - minRadius < range / 2) {
      // treat this as a circle
      Vec2d closestPos = tPos - Vec2d(tPos - aPos, range + minRadius);
      positionsOnCircle(tPos, closestPos, *atk->sod.radius * 2, atkPos);
    }
    else {
      // treat as a real rectangle
      positionsOnRectangle
      ( *atk->sod.x,
        *atk->sod.y,
        *tgt->sod.x1, 
        *tgt->sod.y1, 
        *tgt->sod.x2, 
        *tgt->sod.y2,
        range,
        atkRadius * 2,
        atkPos );
    }
  }
  else {
    int tgtRadius = *tgt->sod.radius;
    Vec2d closestPos = tPos - Vec2d(tPos - aPos, range + tgtRadius);
    positionsOnCircle(tPos, closestPos, *atk->sod.radius * 4, atkPos);
  }

  for(list<Vec2d>::iterator
      i  = atkPos.begin();
      i != atkPos.end();
      i++) 
  {
    list<GameObj*> collisions;
    Vec2d intPos = i->roundToward(tPos);
    if (0 <= intPos(0) && intPos(0) <= Sorts::OrtsIO->getMapXDim() && 
        0 <= intPos(1) && intPos(1) <= Sorts::OrtsIO->getMapYDim()) 
    {
      if (Sorts::spatialDB->hasObjectCollision((int)intPos(0),(int)intPos(1),atkRadius)) {
    //      or 
     //       Sorts::spatialDB->hasTerrainCollision
     //         (intPos(0), intPos(1), atkRadius)
         continue;
      }
      bool slotTaken = false;
      for(list<AttackFSM*>::iterator
          j =  team.begin();
          j != team.end();
          j++) 
      {
        if ((*j)->isMoving()) {
          double d = (intPos - (*j)->getDestination()).magSq();
          double r = *(*j)->getGob()->sod.radius + atkRadius;
          if (d < r * r) {
            slotTaken = true;
            break;
          }
        }
      }
      if (!slotTaken) {
        positions.push_back(intPos);
      }
    }
  //positions.push_back(intPos);
  }
}

AttackManager::AttackManager(const set<SoarGameObject*>& _targets)
: targetSet(_targets)
{
  for(set<SoarGameObject*>::iterator
      i  = targetSet.begin();
      i != targetSet.end();
      ++i)
  {
    targets.insert(pair<SoarGameObject*, AttackTargetInfo>(*i, AttackTargetInfo(*i)));
    idSet.insert((*i)->getID());
  }
  reprioritize();

#ifdef USE_CANVAS_ATTACK_MANAGER
  Uint8 r = (Uint8) (((int) this) % 156) + 100;
  for(set<SoarGameObject*>::iterator
      i  = targetSet.begin();
      i != targetSet.end();
      ++i)
  {
    if (!Sorts::canvas.gobRegistered((*i)->getGob())) {
      Sorts::canvas.registerGob((*i)->getGob());
      Sorts::canvas.setColor((*i)->getGob(), r, 0, 0);
    }
  }
#endif
}

AttackManager::~AttackManager() {
  int status;
  if (targets.size() == 0) {
    status = 1;
  }
  else {
    status = 0;
  }

  for(list<AttackFSM*>::iterator
      i =  team.begin();
      i != team.end();
      ++i)
  {
#ifdef USE_CANVAS_ATTACK_MANAGER
    Sorts::canvas.unregisterGob((*i)->getGob());
#endif
    (*i)->disown(status);
  }
}

void AttackManager::registerFSM(AttackFSM* fsm) {
  team.push_back(fsm);
#ifdef USE_CANVAS_ATTACK_MANAGER
  Sorts::canvas.registerGob(fsm->getGob());
  Uint8 b = (Uint8) (((int) this) % 156) + 100;
  Sorts::canvas.setColor(fsm->getGob(), 0, 0, b);
#endif
}

void AttackManager::unregisterFSM(AttackFSM* fsm) {
  assert(find(team.begin(), team.end(), fsm) != team.end());
  team.erase(find(team.begin(), team.end(), fsm));
  if (fsm->target != NULL) {
    unassignTarget(fsm);
  }

#ifdef USE_CANVAS_ATTACK_MANAGER
  Sorts::canvas.unregisterGob(fsm->getGob());
#endif
  if (team.size() == 0) {
    msg << "I've gone out the window (Nobody cares about me anymore)" << endl;
    Sorts::amr->removeManager(this);
    delete this;
  }
}

void AttackManager::assignTarget(AttackFSM* fsm, SoarGameObject* target) {
  assert(targets.find(target) != targets.end());
  targets[target].assignAttacker(fsm);
  fsm->target = target;
}

void AttackManager::unassignTarget(AttackFSM* fsm) {
  assert(fsm->target != NULL);
  targets[fsm->target].unassignAttacker(fsm);
  fsm->target = NULL;
}

void AttackManager::unassignAll(SoarGameObject* target) {
  assert(targets.find(target) != targets.end());

  AttackTargetInfo& info = targets[target];
  for(set<AttackFSM*>::const_iterator
      i  = info.attackers_begin();
      i != info.attackers_end();
      ++i)
  {
    (*i)->target = NULL;
    (*i)->reassign = true;
  }
  info.unassignAll();
}

bool AttackManager::findTarget(AttackFSM* fsm) {
  msg << "FINDING A TARGET" << endl;
  GameObj* gob = fsm->getGob();
  vector<SoarGameObject*> saturated;
  vector<SoarGameObject*> unsaturated;

  for(vector<SoarGameObject*>::iterator
        i  = sortedTargets.begin();
        i != sortedTargets.end();
        ++i)
  {
    if (targets[*i].isSaturated()) {
      saturated.push_back(*i);
    }
    else {
      unsaturated.push_back(*i);
    }
  }

/*
  for(int checkSaturated = 1; checkSaturated >= 0; checkSaturated--) {
    vector<SoarGameObject*>* toCheck;
    if (checkSaturated == 0) {
      toCheck = &saturated;
    }
    else {
      toCheck = &sortedTargets;
    }
*/
  // try to hit immediately attackable things first
    for(vector<SoarGameObject*>::iterator
        i  = sortedTargets.begin();
        i != sortedTargets.end();
        ++i)
    {
//      if (canHit(gob, (*i)->getGob()) && 
//          (checkSaturated == 0 || !targets[*i].isSaturated()))
      if (canHit(gob, (*i)->getGob())) {
        assignTarget(fsm, *i);
#ifdef USE_CANVAS_ATTACK_MANAGER
        GameObj* tgob = (*i)->getGob();
        Sorts::canvas.trackDestination(gob, gobX(tgob), gobY(tgob));
#endif
        msg << "NEARBY TARG" << endl;
        return true;
      }
    }

  // now try to attack the "best" target
//  for(int checkSaturated = 1; checkSaturated >= 0; checkSaturated--) {
    for(vector<SoarGameObject*>::iterator
      i  = sortedTargets.begin();
      i != sortedTargets.end();
      ++i)
    {
//      if (checkSaturated == 0 || !targets[*i].isSaturated()) {
        list<Vec2d> positions;
        attackArcPos(fsm->getGob(), (*i)->getGob(), 0, positions);
        for(list<Vec2d>::iterator
            j  = positions.begin();
            j != positions.end();
            ++j)
        {
          if (fsm->move((int)(*j)(0), (int)(*j)(1)) == 0) {
            msg <<"Moving to Position: "<<(*j)(0)<<", "<<(*j)(1)<<endl;
            assignTarget(fsm, *i);
#ifdef USE_CANVAS_ATTACK_MANAGER
            GameObj* gob = (*i)->getGob();
            Sorts::canvas.trackDestination(fsm->getGob(),*gob->sod.x,*gob->sod.y);
#endif
            msg << "ARC TARG" << endl;
            return true;
          }
          else {
            msg << "ARC CHECK MOVE FAIL" << endl;
          }
        }
//      }
    }
//  }

  // finally, just run toward someone, preferrably unsaturated
//  for(int checkSaturated = 1; checkSaturated >= 0; checkSaturated--) {
    for(vector<SoarGameObject*>::iterator
        i  = sortedTargets.begin();
        i != sortedTargets.end();
        ++i)
    {
//      if (checkSaturated == 0 || !targets[*i].isSaturated()) {
        GameObj* tgob = (*i)->getGob();
        Vec2d closePos = getClosePos(gob, tgob);
        if (fsm->move((int)closePos(0), (int)closePos(1)) == 0) {
          assignTarget(fsm, *i);
          msg << "LAST RESORT TARG" << endl;
          return true;
        }
//      }
    }
//  }
//  }
  return false;
}

// the current strategy is basically to focus fire on one enemy
// at a time until they're all dead, minimizing damage taken by self

// In the future, also implement running weak units away
int AttackManager::direct(AttackFSM* fsm) {
  msg << "NUMTARGS: " << targets.size() << endl;
  unsigned long st = gettime();
#ifdef USE_CANVAS_ATTACK_MANAGER
  Sorts::canvas.flashColor(fsm->getGob(), 0, 255, 0, 1);
  Sorts::canvas.update();
#endif

  GameObj* gob = fsm->getGob();
  SoarGameObject* sgob = Sorts::OrtsIO->getSoarGameObject(gob);

  if ( gob->dir_dmg > 0 && 
       ((double) gob->get_int("hp")) / gob->get_int("max_hp") < 0.2)
  {
    // time to run away
    for(int i = 0; i < GameConst::DAMAGE_N; ++i) {
      if (damageTaken(i, gob->dir_dmg)) {
        Vec2d retreatVector = getHeadingVector(i).inv();
        Vec2d moveDest = Vec2d(gobX(gob), gobY(gob)) + Vec2d(retreatVector, 10);
        fsm->move((int)moveDest(0), (int)moveDest(1));
        msg << "RETREATING TO " << moveDest(0) << ", " << moveDest(1) << endl;
        return 0;
      }
    }
  }

  int numUnassigned = 0;
  for(list<AttackFSM*>::iterator
      i  = team.begin();
      i != team.end();
      ++i)
  {
    if ((*i)->target == NULL) {
      numUnassigned++;
    }
  }
  msg << "UNASSIGNED: " << numUnassigned << endl;


  if (updateTargetList() > 0) {
    msg << "UPDATE TARGET LIST" << endl;
    if (targets.size() == 0) {
      Sorts::amr->removeManager(this);
      delete this;
      return 1;
    }
    reprioritize();

    /*
    for(list<AttackFSM*>::iterator
        i  = team.begin();
        i != team.end();
        ++i)
    {
      if ((*i)->target != NULL) {
        unassignTarget(*i);
      }
    }
    */
  }

  if (fsm->target == NULL) {
    unsigned long st_find = gettime();
    if (!findTarget(fsm)) {
      msg << "FIND TARGET FAILED" << endl;
      fsm->failCount++;
      if (fsm->failCount > 10) {
        return -1;
      }
      return 0;
    }
    msg << "TIME TO FIND TARGET: " << (gettime() - st_find) / 1000 << endl;
  }
  
  assert(fsm->target != NULL);

  if (idSet.find(fsm->target->getID()) == idSet.end()) {
    msg << "bad target Ptr: " <<(int) fsm->target->getGob() << " id: " << fsm->target->getID() << " name: " << fsm->target->getGob()->bp_name() << endl;
    msg << "bad target sgo ptr: " << (int)fsm->target << endl;
  }

  msg << "attacking target: " << fsm->target->getGob()->bp_name() << " " 
      << fsm->target->getGob() << " aka sgo " << (int)fsm->target << endl;

  fsm->failCount = 0;

  GameObj* tgob = fsm->target->getGob();
  AttackTargetInfo& info = targets[fsm->target];

  if (!canHit(gob, tgob)) {
    if (fsm->isMoving()) {
      Vec2d dest = fsm->getDestination();
      if (canHit(gob, dest, tgob)) {
        if (gob->dir_dmg == 0) {
          double distToTarget 
            = squaredDistance(gobX(gob), gobY(gob), gobX(tgob), gobY(tgob));
          if (distToTarget < info.avgAttackerDistance() * WAIT_RATIO)
          {
            msg << "WAITING FOR OTHERS TO CATCH UP" << endl;
#ifndef NO_WAITING
            fsm->waitingForCatchup = true;
#endif
          }
        }
        // everything's fine, keep going
        msg << "TIME SPENT: " << (gettime() - st) / 1000 << endl;
        return 0;
      }
      else {
        msg << "xxx_dest can't hit. Dest: " << dest(0) << "," << dest(1) << " Targ: " << gobX(tgob) << "," << gobY(tgob) << endl;
      }
    }
    if (fsm->waitingForCatchup) {
      msg << "WAITING" << endl;
      if ((gob->dir_dmg == 0) && (canHit(gob, fsm->getDestination(), tgob))) {
        double distToTarget 
          = squaredDistance(gobX(gob), gobY(gob), gobX(tgob), gobY(tgob));
        if (distToTarget >= info.avgAttackerDistance() * RESUME_RATIO) {
          fsm->waitingForCatchup = false;
        }
        else {
          // keep waiting
          msg << "TIME SPENT: " << (gettime() - st) / 1000 << endl;
          return 0;
        }
      }
      else {
        // forget you guys, I'm getting out of here
        fsm->waitingForCatchup = false;
      }
      // otherwise, he's taking damage, or the target changed
      // position, so start moving
    }
    // not moving, or should be moving somewhere else
    msg << "CANNOT HIT" << endl;

    list<Vec2d> positions;
    attackArcPos(gob, tgob, 0, positions);
    for(list<Vec2d>::iterator
        i  = positions.begin();
        i != positions.end();
        ++i)
    {
      if (fsm->move((int)(*i)(0), (int)(*i)(1)) == 0) {
        msg << "Moving to Position: " << (*i)(0) << ", " << (*i)(1) << endl;
#ifdef USE_CANVAS_ATTACK_MANAGER
//        Sorts::canvas.trackDestination(fsm->getGob(), (*i)(0), (*i)(1));
#endif
        break;
      }
      else {
        msg << "ARC CHECK MOVE FAIL 2" << endl;
      }
    }
    if (!fsm->isMoving()) {
      fsm->target = NULL;
    }
  }
  else if (!fsm->isFiring() ||  
           sgob->getLastAttacked() != fsm->target->getID())
  {
    fsm->attack(fsm->target);
  }

  msg << "TIME SPENT: " << (gettime() - st) / 1000 << endl;
  return 0;
}

int AttackManager::updateTargetList() {
  int numVanished = 0;
  map<SoarGameObject*, AttackTargetInfo>::iterator i = targets.begin();
  while (i != targets.end()) {
    if (!Sorts::OrtsIO->isAlive(i->first->getID())) {
      msg << "(" << (int) this << ") Unit " << i->first->getID() << " is no longer alive or moved out of view" << endl;

#ifdef USE_CANVAS_ATTACK_MANAGER
      // this target could have been in multiple attack managers
      if (Sorts::canvas.gobRegistered(i->first->getGob())) {
        Sorts::canvas.unregisterGob(i->first->getGob());
      }
#endif
      unassignAll(i->first);
      targetSet.erase(i->first);
      map<SoarGameObject*, AttackTargetInfo>::iterator j = i++;
      targets.erase(j);
      ++numVanished;
    }
    else {
      ++i;
    }
  }
  return numVanished;
}

struct TargetCompare {
  Vec2d myPos;

  bool operator()(SoarGameObject* t1, SoarGameObject* t2) {
    // first always prefer those that are armed and shooting over 
    // those that are not
    ScriptObj* weapon1 = t1->getGob()->component("weapon");
    ScriptObj* weapon2 = t2->getGob()->component("weapon");
    if (weapon1 != NULL && weapon2 == NULL) {
      return true;
    }
    if (weapon1 == NULL && weapon2 != NULL) {
      return false;
    }
    
    if (weapon1 != NULL && weapon2 != NULL) {
      if (weapon1->get_int("shooting") == 1 && 
          weapon2->get_int("shooting") == 0) 
      {
        return true;
      }
      if (weapon1->get_int("shooting") == 0 && 
          weapon2->get_int("shooting") == 1) 
      {
        return false;
      }
    }

    // this formula was derived by minimizing damage to your own units,
    // and assuming that none of your units die while attacking (your
    // damage rate stays constant)
    double rating1 = weaponDamageRate(t1->getGob()) * t2->getGob()->get_int("hp");
    double rating2 = weaponDamageRate(t2->getGob()) * t1->getGob()->get_int("hp");

    if (rating1 < rating2) {
      return true;
    }
    else if (rating1 == rating2) { // break ties by distance
      Vec2d p1(*t1->getGob()->sod.x, *t1->getGob()->sod.y);
      Vec2d p2(*t2->getGob()->sod.x, *t2->getGob()->sod.y);

      double dist1 = (myPos - p1).magSq();
      double dist2 = (myPos - p2).magSq();

      if (dist1 < dist2) {
        return true;
      }
      else if (dist1 == dist2) {  // lexicographically break ties
        return (t1->getID() < t2->getID());
      }
    }
    return false;
  }
};

void AttackManager::reprioritize() {
  // calculate centroid
  double xsum = 0, ysum = 0;
  for(list<AttackFSM*>::iterator
      i  = team.begin();
      i != team.end();
      i++)
  {
    xsum += *(*i)->getGob()->sod.x;
    ysum += *(*i)->getGob()->sod.y;
  }
  
  TargetCompare comparator;
  comparator.myPos = Vec2d(xsum / team.size(), ysum / team.size());

  sortedTargets.clear();
  sortedTargets.insert(sortedTargets.end(), targetSet.begin(), targetSet.end());
  sort(sortedTargets.begin(), sortedTargets.end(), comparator);
}
