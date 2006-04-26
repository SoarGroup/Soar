#ifndef InternalGroup_h
#define InternalGroup_h

#include <list>
#include <set>
#include <string>
#include "general.h"

class Sorts;
class SoarGameObject;
class FeatureMap;

using namespace std;

class InternalGroup {
  public:
    InternalGroup
    ( SoarGameObject* unit);

    ~InternalGroup();

    void addUnit(SoarGameObject* unit);
    bool removeUnit(SoarGameObject* unit);
    void updateCenterLoc();
    bool isEmpty();

    list<SoarGameObject*> getMembers(); 
  
    void mergeTo(InternalGroup* target);
    bool getHasStaleMembers();
    void setHasStaleMembers();
    void setHasStaleMembers(bool val);
    pair<string, int> getCategory();
    int getSize();
    SoarGameObject* getNextMember();
    void getCenterLoc(int& x, int& y); 

  private:
    set <SoarGameObject*> members;
    
    bool hasStaleMembers;

    string typeName;
    
    SoarGameObject* currentMember; // for getNextMember functionality

    int owner;

    int centerX, centerY;
};

#endif
