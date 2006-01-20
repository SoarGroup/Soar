#ifndef SoarGameGroup_h
#define SoarGameGroup_h

#include <set>

class SoarGameGroup {
  public:
    SoarGameGroup(SoarGameObject* unit);
    void addUnit(SoarGameObject* unit);
    bool removeUnit(SoarGameObject* unit);
    void updateStats();
    bool assignAction(Action);
  private:
    set <SoarGameObject*> members;
    // int capabilities; // get from unit capabilities
};
#endif
