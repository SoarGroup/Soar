#include <assert.h>
#include "SoarInterface.h"

using namespace sml;

SoarInterface::SoarInterface() {
  groupIdCounter = 0;
}

SoarInterface::~SoarInterface() {
}

/* Note: We don't need to create the property list associated with
 * the group at this time, wait until a refresh
 */
void SoarInterface::addGroup(SoarGameGroup* group) {
  // make sure the group does not exist already
  assert(soarGroups.find(group) == soarGroups.end());
  
  SoarIOGroupRep newGroup;
  newGroup.groupId = -1;
  
  soarGroups[group] = newGroup;
}

void SoarInterface::removeGroup(SoarGameGroup* group) {
  // make sure the group exists
  assert(soarGroups.find(group) != soarGroups.end());
  
  SoarIOGroupRep &g = soarGroups[group];
  if (g.groupId >= 0) {
    agent->DestroyWME(g.WMEptr);
  }
  soarGroups.erase(group);
}

void SoarInterface::refreshGroup(SoarGameGroup* group, groupPropertyList& gpl) {
  // make sure the group exists
  assert(soarGroups.find(group) != soarGroups.end());
  
  SoarIOGroupRep &g = soarGroups[group];
  
  if (g.groupId < 0) {
    // add the group to the soar input link if it hasn't been already
    g.groupId = groupIdCounter++;
    g.WMEptr = agent->CreateIdWME(inputLink, "group");
    for(groupPropertyList::iterator i = gpl.begin(); i != gpl.end(); i++) {
      for(groupPropertyList::iterator i = gpl.begin(); i != gpl.end(); i++) {
        // create a new WME object for the property
        g.properties[(*i).first] = agent->CreateIntWME(g.WMEptr, (*i).first.c_str(), (*i).second);
      }
    }
  }
  else {
    // group already added, just update values.
    // Note that I'm assuming no new values are introduced
    for(groupPropertyList::iterator i = gpl.begin(); i != gpl.end(); i++) {
      agent->Update(g.properties[(*i).first], (*i).second);
    }
  }
}
