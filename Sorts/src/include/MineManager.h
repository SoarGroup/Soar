#ifndef MINEMANAGER_H
#define MINEMANAGER_H


#include "general.h"
#include "SoarGameObject.h"
class MineFSM;
class Sorts;
/* 
  MineManager.h 
  SORTS Project 
  Sam Wintermute, 2006 
*/ 

enum Direction {
  NORTH=0,
  SOUTH=1,
  EAST=2,
  WEST=3
};

#define NO_DOS
/* NO_DOS definition:
   This defines "no dropoffs" mode. When this is not defined, each mining
   route consists of two points, a mining location and a dropoff location.
   With NO_DOS defined, the routes consist only of a mining location, and 
   the location of the controlCenter to go to. The difference is that in the
   first case, the worker has an exact coordinate to go to, while in the second
   the worker simply moves towards the command center until it runs into it.
   Using dropoff locations can make mining more efficient (and should be used if
   that is all the agent is doing), but the calculation of the routes is much
   more expensive. So, in most cases, NO_DOS should be defined.
 */


enum RouteHeuristicStage {
  // this is the heuristic used to determine path optimality
  
  // first, just use the straight-line distance cCenter->mineral object
  STRAIGHT_LINE_DIST,
  // if the route is selected, do a pathfind from the cc to the mineral,
  // and take the total path length as the new optimality
  OBJ_OBJ_PF_DIST,
  // if that is selected, the route must be expanded to 16 routes, each from a
  // different edge of the cc to a different mineral edge.
  // for each of these, pathfind for the edge-edge distance
  EDGE_EDGE_PF_DIST,
  // then, if it still appears best, the edge-edge routes need to be expanded
  // to ~20 point-to-point route, with
  // the actual pathfind distance between the points
  PF_DIST,

  // when an edge-edge route is expanded, we can save lots of time if we
  // don't pf new routes where a station is in use, since we don't want to 
  // double up on routes. but when workers leave routes, stations open up
  // and the affected routes should be reconsidered
  STATION_IN_USE
};

#define CCENTER_MAXRADIUS 44
#define MINERAL_RADIUS 6
#define WORKER_RADIUS 3
#define CCENTER_MINRADIUS 31

struct StationInfo;
struct MineralInfo;
struct CCenterInfo;
struct MiningRoute {
  coordinate miningLoc;
  coordinate dropoffLoc;
  MineralInfo* mineralInfo;
  CCenterInfo* cCenterInfo;
  list<MineFSM*> fsms;
  double optimality;
  double pathlength;
  RouteHeuristicStage stage;
  StationInfo* mineStation;
  StationInfo* dropoffStation;
  bool valid;
};

#define MINERAL_EDGE_STATIONS 2
#define CC_EDGE_STATIONS 10

struct MineralInfo {
  SoarGameObject* mineral;
  list<MiningRoute*> routes;
  bool stationsValid[4];
  StationInfo* northStations[MINERAL_EDGE_STATIONS];
  StationInfo* southStations[MINERAL_EDGE_STATIONS];
  StationInfo* eastStations[MINERAL_EDGE_STATIONS];
  StationInfo* westStations[MINERAL_EDGE_STATIONS];
};

struct CCenterInfo {
  SoarGameObject* cCenter;
  list <MiningRoute*> routes;
  bool stationsValid[4];
  StationInfo* northStations[CC_EDGE_STATIONS];
  StationInfo* southStations[CC_EDGE_STATIONS];
  StationInfo* eastStations[CC_EDGE_STATIONS];
  StationInfo* westStations[CC_EDGE_STATIONS];
};

struct StationInfo {
  int optimality;
  coordinate location;
  list <MiningRoute*> routes;
};

struct ltMiningRoutePtr {
  bool operator()(MiningRoute* g1, MiningRoute* g2) const {
    double d1 = g1->optimality;
    double d2 = g2->optimality;
    if (d1 == d2) {
      // give an arbitrary order if optimality is the same
      return ((int)g1 < (int)g2);
    }
    return (d1 < d2);
  }
};

struct ltMineralInfoPtr {
  // use this so we can have a set of mineralInfo's, but do quick lookups
  // based on the mineral object alone
  bool operator()(MineralInfo* g1, MineralInfo* g2) const {
    return ((int)(g1->mineral) < (int)(g2->mineral));
  }
};
class MineManager {
  public:
    MineManager();
    ~MineManager();

    // called by the group when it recieves a mine command.
    // select the n best routes, and find which miner is closest to each
    void prepareRoutes(list<SoarGameObject*>& miners);

    // called by MineFSM to get the path it should take to the mining location
    // it is assumed that the given FSM will be working on this until it calls
    // reportMineTime.
    MiningRoute* getMiningRoute(MineFSM* fsm);
    
    MiningRoute* minerGivesUp(MiningRoute*, MineFSM*); 

    // called by OrtsInterface when minerals appear and disappear
    void addMineral(SoarGameObject* mineral);
    void removeMineral(SoarGameObject* mineral);

    // likewise
    void addControlCenter(SoarGameObject* center);
    void removeControlCenter(SoarGameObject* center);

    // called by MineFSM destructor if a worker dies
    void removeWorker(MiningRoute* route, MineFSM* fsm);
    
  private:
    int newRoutes;
    int maxX;
    int maxY;
    // store minerals here, for quick lookup when they disappear
    // MineralInfo connects minerals to routes
    set<MineralInfo*, ltMineralInfoPtr> minerals;
    
    // store commandCenters here- they rarely change, so a list is fine
    // CCenterInfo connects cCenters to routes
    list<CCenterInfo*> cCenters;
    
    // routes- keep the most optimal at the top
    set<MiningRoute*, ltMiningRoutePtr> routes;
    
    // storage for the sgo->route assignments
    map<SoarGameObject*, MiningRoute*> assignments;

    // keep station ptrs somewhere so we can delete them
    list<StationInfo*> allStations;

    // routes to be deleted-
    // can't delete until the end because they may be in lists in various
    // places (attached to minerals, etc.)
    list<MiningRoute*> invalidRoutes;
    
    void removeFromRoute(MiningRoute* route, MineFSM* fsm);
    void removeFromRouteNoErase(MiningRoute* route, list<MineFSM*>::iterator it);
    void addFSMToRoute(MiningRoute* route, MineFSM* fsm);
    void addCostToRoute(MiningRoute* route);
    void adjustOptimality(MiningRoute* route);
    void calculateOptimality(MiningRoute* route);
    void addRoute(CCenterInfo* cci, MineralInfo* mi);
    MiningRoute* getBestRoute();
    void expandSLD(MiningRoute* route);
    void expandObjObj(MiningRoute* route);
    void expandEdgeEdge(MiningRoute* route);
    void expandSIU(MiningRoute* route);
    double pathFindDist(SoarGameObject* obj1, SoarGameObject* obj2);
    double pathFindDist(coordinate loc1, coordinate loc2, bool b);
    void allocateMiningStations(MineralInfo* m, Direction d);
    void allocateDropoffStations(CCenterInfo* m, Direction d);
    Direction getRelDirection(coordinate c1, coordinate c2);
    bool collision(StationInfo* station);
};
#endif
