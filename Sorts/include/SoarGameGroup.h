#ifndef SoarGameGroup_h
#define SoarGameGroup_h
class SoarGameGroup {
  public:
    SoarGameGroup(SoarGameObject* unit);
    void addUnit(SoarGameObject* unit);
    bool removeUnit(SoarGameObject* unit);
    void updateStats();
    bool assignAction(Action);
  private:
    // ???
};
#endif
