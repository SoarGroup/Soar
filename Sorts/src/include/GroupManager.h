#ifndef GroupManager_h
#define GroupManager_h

class GroupManager {
  public:
    GroupManager(SoarInterface* si, ORTSInterface* oi) 
      : SoarIO(si), ORTSIO(oi) { };
    ~GroupManager();

    void updateWorld();
    bool assignActions();
  
  private:
    void createNewGroups();
    void adjustGroups();
    void updateStats();

    SoarInterface* SoarIO;
    ORTSInterface* ORTSIO;
    
    vector <SoarGameGroup*> groupsInFocus;
    vector <SoarGameGroup*> groupsNotInFocus;
    vector <SoarGameGroup*> newGroups;
}

#endif // GroupManager_h
