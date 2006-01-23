#include "SoarGameGroup.h"
#include <assert.h>
#include <vector>

SoarGameGroup::SoarGameGroup(SoarGameObject* unit) {
  members.clear();
  members.push_back(unit);
  // capabilities = unit->capabilities;
}

SoarGameGroup::addUnit(SoarGameObject* unit) {
  //capabilities &= unit->capabilities;
  assert(not members.find(unit));
  members.insert(unit);
}

SoarGameGroup::removeUnit(SoarGameObject* unit) {
  assert(members.find(unit));
  members.erase(unit);
}

groupPropertyList SoarGameGroup::updateStats() {
  groupPropertyList propList;
  // string / int pairs to send to Soar
  
  set<SoarGameObject*>::iterator currentObject = members.begin();
  
  double health = 0;
  double x = 0;
  double y = 0;
  double size = members.size();

  while (currentObject != members.end()) {
    // be careful is some numbers are very big for each object and the double could overflow
    
    health += *currentObject.getHealth();
    x += *currentObject.getX();
    y += *currentObject.getY();
  
    currentObject++;
  }
  
  health /= size;
  x /= size;
  y /= size;

  statistics[GP_NUM_STATS] = size;
  statistics[GP_X_POS] = x;
  statistics[GP_Y_POS] = y;
  statistics[GP_HEALTH] = health;

  pair<string, int> wme;
  wme.first = "health";
  wme.second = health;
  propList.push_back(wme);

  // how do we want to represent positon?
  wme.first = "x_position";
  wme.second = x;
  propList.push_back(wme);
  wme.first = "y_position";
  wme.second = y;
  propList.push_back(wme);

  return propList;
}
