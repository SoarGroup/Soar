#ifndef soarinterface_h
#define soarinterface_h
typedef list<pair<string, int> > groupPropertyList;

class SoarInterface{
  public:
    SoarInterface();
    ~SoarInterface();
    vector <SoarAction*> getNewActions();
    bool addGroup(SoarGameGroup* group);
    bool removeGroup(SoarGameGroup* group);
    bool refreshGroup(SoarGameGroup* group, groupPropertyList& gpl);
  private:
    // ??
};


/* The GroupManager will have a pointer to this structure, and can call the public functions to get new actions and change what groups Soar can "see". Whatever needs to be done to initialize Soar or do higher level things like pause for debugging will be done by the main loop, which can call other (not yet defined) functions inside the SoarInterface.

SoarInterface will be responsible for creating and organizing the WMEs for the groups, but not for determining what information is in the structure (for example, we don't want a "health" WME for a tree, but SoarInterface shouldn't need to figure that out). There will be a list of visible properties inside the SoarGameGroups that will be set by the GroupManager and read by the SoarInterface to determine that.

Note: addGroup should not actually add the group to the input_link until that group is refreshed! Initially, the stats will not be set. */
#endif
