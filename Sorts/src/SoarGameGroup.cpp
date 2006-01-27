#include "include/SoarGameGroup.h"
#include <assert.h>
#include <vector>

SoarGameGroup::SoarGameGroup(SoarGameObject* unit) {
  members.clear();
  members.insert(unit);
  // capabilities = unit->capabilities;
  stale = true;
}

void SoarGameGroup::addUnit(SoarGameObject* unit) {
  //capabilities &= unit->capabilities;

  /* Sam: You can't use the line below, because find returns
   *      an iterator. Instead you have to use what I've changed
   *      it to.
   *
   *      - Joseph
   */
  //assert(not members.find(unit));
  assert(members.find(unit) == members.end());
  members.insert(unit);
  stale = true;
}

bool SoarGameGroup::removeUnit(SoarGameObject* unit) {
  //assert(members.find(unit));
  assert(members.find(unit) != members.end());

  members.erase(unit);
  stale = true;
  return true;
}

groupPropertyList SoarGameGroup::updateStats() {
  groupPropertyList propList;
  // string / int pairs to send to Soar
  
  set<SoarGameObject*>::iterator currentObject = members.begin();
  
  double health = 0;
  double x = 0;
  double y = 0;
  double size = members.size();

  // this is not a huge problem (just a redundant update), but should not happen
  assert(stale == true);
  
  while (currentObject != members.end()) {
    // be careful is some numbers are very big for each object and the double could overflow
    
  //  health += *currentObject.getHealth();
  //  x += *currentObject.getX();
  //  y += *currentObject.getY();
  
    currentObject++;
  }
  
  health /= size;
  x /= size;
  y /= size;

  statistics[GP_NUM_MEMBERS] = size;
  statistics[GP_X_POS] = x;
  statistics[GP_Y_POS] = y;
  statistics[GP_HEALTH] = health;

  pair<string, int> wme;
  wme.first = "health";
  wme.second = (int)health;
  propList.push_back(wme);

  // how do we want to represent positon?
  wme.first = "x_position";
  wme.second = (int)x;
  propList.push_back(wme);
  wme.first = "y_position";
  wme.second = (int)y;
  propList.push_back(wme);

  stale = false;
  return propList;
}

void SoarGameGroup::mergeTo(SoarGameGroup* target) {
  // move all members into other group

  // the group should be destructed after this is called.

  
  set<SoarGameObject*>::iterator currentObject = members.begin();
  
  while (currentObject != members.end()) {
    target->addUnit(*currentObject);
    currentObject++;
  }

  members.clear();
  statistics[GP_NUM_MEMBERS] = 0;
  stale = true;

  return;
}
