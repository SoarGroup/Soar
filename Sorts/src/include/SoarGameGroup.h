#ifndef SoarGameGroup_h
#define SoarGameGroup_h

#include <list>
#include <set>
#include <string>
#include "Action.h"
//#include "OrtsInterface.h"
#include "general.h"
#include "MapRegion.h"
#include "MapManager.h"
#include "Rectangle.h"

class OrtsInterface;
class SoarGameObject;
class FeatureMap;

using namespace std;

//class SoarGameObject{}; // TEMPORARY

class SoarGameGroup {
  public:
    SoarGameGroup( SoarGameObject* unit,
                   bool _mixedType,
                   OrtsInterface*  _ORTSIO,
                   MapManager*     _mapManager );

    ~SoarGameGroup();

    void addUnit(SoarGameObject* unit);
    bool removeUnit(SoarGameObject* unit);
    void generateData();
    bool assignAction(ObjectActionType type, list<int> params,
                      list<SoarGameGroup*> targets);
    bool isEmpty();

    list<SoarGameObject*> getMembers(); 
  
    void mergeTo(SoarGameGroup* target);
    bool getHasStaleMembers();
    void setHasStaleMembers();
    void setHasStaleMembers(bool val);
    bool getHasStaleProperties();
    void setHasStaleProperties(bool val);
    groupPropertyStruct getSoarData();
    pair<string, int> getCategory();
    int getSize();
    SoarGameObject* getCenterMember();
    SoarGameObject* getNextMember();
    void updateCenterMember();

    // get the player number that owns this group
    int getOwner();
    bool isWorld();
    bool isFriendly();

    bool isMinerals();
    bool isAirUnits();
    bool isLandUnits();
    bool isMoving();

    void setFMSector(int);
    int getFMSector();

    void setFMaps(list <FeatureMap*>);
    list <FeatureMap*> getFMaps();
    
    void setFMFeatureStrength(int);
    int getFMFeatureStrength();

    Rectangle getBoundingBox();
    void getCenterLoc(int& x, int& y);

    // return a point (locX, locY) on the bounding box of the group
    // (or something similar) that is close to x, y
    void getLocNear(int x, int y, int& locX, int &locY);
    
    bool getSticky();
    void setSticky(bool in);

  private: // functions
    void updateBoundingBox();
    void updateRegionsOccupied();

  private:
    set <SoarGameObject*> members;
    groupPropertyStruct soarData;
    bool hasStaleMembers;
    bool hasStaleProperties;
    // staleMembers means the internal statistics haven't been updated-
    // something moved, or new members were added.

    // staleProperties means that internal statistics are correct, but Soar
    // needs a refresh-- either refresh it or remove it from Soar ASAP

    string typeName;
    
    SoarGameObject* centerMember;
    SoarGameObject* currentMember; // for getNextMember functionality

    int owner;
    bool friendly;
    bool world;
    bool minerals;
    bool airUnits;
    bool landUnits;
    bool moving;
    OrtsInterface* ORTSIO;

    // bounding box of group dimensions
    Rectangle bbox;

    int centerX, centerY;

    MapManager* mapManager;
    list<MapRegion*> regionsOccupied;

    // sticky if grouped by Soar-
    // issuing a command makes it auto-sticky until command ends & Soar acks
    bool sticky;

    // info on the last command executed- what is it, and is it done?
    string currentCommand;
    int commandStatus;

    bool canMine;
    
    // true if there is more than one type of unit in the group
    bool mixedType;
    
    // feature map support-
    // the group needs to remember what feature maps it is in, and the sector
    int fmSector;
    list <FeatureMap*> fMaps;
    int fmFeatureStrength;
};

#define GRP_STATUS_RUNNING 0
#define GRP_STATUS_SUCCESS 1
#define GRP_STATUS_FAILURE 2
#define GRP_STATUS_IDLE 3
#endif
