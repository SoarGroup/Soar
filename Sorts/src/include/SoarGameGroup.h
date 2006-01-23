#ifndef SoarGameGroup_h
#define SoarGameGroup_h

#include <list>
#include <set>

class SoarGameGroup {
  public:
    SoarGameGroup(SoarGameObject* unit);
    void addUnit(SoarGameObject* unit);
    bool removeUnit(SoarGameObject* unit);
    groupPropertyList updateStats();
    bool assignAction(Action);
    void getUnits(list<SoarGameObject*> unitList);
  private:
    set <SoarGameObject*> members;
    // int capabilities; // get from unit capabilities
    vector <int> statistics; 
};
#endif
