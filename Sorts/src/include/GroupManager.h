#ifndef GroupManager_h
#define GroupManager_h

class GroupManager {
  public:
    GroupManager(SoarInterface* si, ORTSInterface* oi) 
      : SoarIO(si), ORTSIO(oi) { };
    ~GroupManager();

    void updateWorld();
    bool assignActions();

    void addGroup(const SoarGameObject* object);
    // used by ORTSInterface when it sees a new object- create a group for it
    
  private:
    void reGroup();
    void updateStats();

    SoarInterface* SoarIO;
    ORTSInterface* ORTSIO;
    
    list <SoarGameGroup*> groupsInFocus;
    list <SoarGameGroup*> groupsNotInFocus;
}

#endif // GroupManager_h
