#ifndef PerceptualGroup_h
#define PerceptualGroup_h

#include <list>
#include <set>
#include <string>

#include "SoarAction.h"
#include "AttributeSet.h"
//#include "OrtsInterface.h"
#include "general.h"
#include "MapRegion.h"
#include "Rectangle.h"

class Sorts;
class SoarGameObject;
class FeatureMap;

using namespace std;

//class SoarGameObject{}; // TEMPORARY

class PerceptualGroup {
  public:
    PerceptualGroup
    ( SoarGameObject* unit);

    ~PerceptualGroup();

    void addUnit(SoarGameObject* unit);
    bool removeUnit(SoarGameObject* unit);
    void generateData();
    bool assignAction(ObjectActionType type, list<int> params,
                      list<PerceptualGroup*> targets);
    bool isEmpty();

    void getMembers(list<SoarGameObject*>& memberList); 
  
    void mergeTo(PerceptualGroup* target);
    bool getHasStaleMembers();
    void setHasStaleMembers();
    void setHasStaleMembers(bool val);
    bool getHasStaleProperties();
    void setHasStaleProperties(bool val);
    const AttributeSet& getAttributes();
    pair<string, int> getCategory(bool ownerGrouping);
    int getSize();
    SoarGameObject* getCenterMember();
    void updateCenterMember();
    void updateCenterLoc();

    // get the player number that owns this group
    int getOwner();
    bool isWorld();
    bool isFriendly();

    bool isMinerals();
    bool isAirUnits();
    bool isLandUnits();
    bool isMoving();
    bool isFriendlyWorker();

    void setFMSector(int);
    int getFMSector();

    void setFMaps(list <FeatureMap*>);
    list <FeatureMap*> getFMaps();
    
    void setFMFeatureStrength(int);
    int getFMFeatureStrength();

    Rectangle& getBoundingBox();
    void getCenterLoc(int& x, int& y);

    // return a point (locX, locY) on the bounding box of the group
    // (or something similar) that is close to x, y
    void getLocNear(int x, int y, int& locX, int &locY);
    
    bool getSticky();
    void setSticky(bool in);

    void calcDistToFocus(int focusX, int focusY);
    double getDistToFocus();

    bool getInSoar();
    void setInSoar(bool val);

    int getSoarID();
    void setSoarID(int);

    // return true if the group has been around for multiple cycles
    // (return false if it was just created)
    // this is set to false initially, set to true when generateData called.
    bool isOld();
    
    void getRegionsOccupied(list<int>& regions);


    bool getHasCommand() { return hasCommand; }
  
    void setCommandString(string st) { currentCommand = st; }
    string getCommandString() { return currentCommand; }

  private: // functions
    void updateBoundingBox();
    void updateRegionsOccupied();

  private:
    set <SoarGameObject*> members;
    AttributeSet attribs;
    SoarGameObject* getMemberNear(coordinate c);

    bool inSoar; // true if present in input link
    
    bool old;
    
    bool hasStaleMembers;
    bool hasStaleProperties;
    // staleMembers means the internal statistics haven't been updated-
    // something moved, or new members were added.

    // staleProperties means that internal statistics are correct, but Soar
    // needs a refresh-- either refresh it or remove it from Soar ASAP

    string typeName;
    
    SoarGameObject* centerMember;

    int owner;
    bool friendly;
    bool world;
    bool minerals;
    bool airUnits;
    bool landUnits;
    bool moving;
    bool friendlyWorker;

    // bounding box of group dimensions
    Rectangle bbox;

    int centerX, centerY;

    list<MapRegion*> regionsOccupied;

    // sticky if grouped by Soar-
    // issuing a command makes it auto-sticky until command ends & Soar acks
    bool sticky;

    // info on the last command
    string currentCommand;
    bool hasCommand;

    bool canMine;
    
    // true if there is more than one type of unit in the group
    bool mixedType;
    
    // feature map support-
    // the group needs to remember what feature maps it is in, and the sector
    int fmSector;
    list <FeatureMap*> fMaps;
    int fmFeatureStrength;

    double distToFocus;
    coordinate lastQueryResult;
    double lastQueryDist;
    int soarID;

};

#endif
