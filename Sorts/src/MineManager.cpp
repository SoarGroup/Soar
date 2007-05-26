#include "MineManager.h"
#include "Sorts.h"
#include "MineFSM.h"

/*
   MineManager.cpp
   Sorts project
   Sam Wintermute, 2006
*/

#define CLASS_TOKEN "MINEMAN"
#define DEBUG_OUTPUT false 
#include "OutputDefinitionsUnique.h"

#define UNUSABLE_OPTIMALITY 1000
#define WAYPOINT_PENALTY 10

#define DROPOFF_COST UNUSABLE_OPTIMALITY
#define MINE_COST UNUSABLE_OPTIMALITY

#define PF_CACHE

#define MAX_NEW_ROUTES 1000

#define SLDOO_OVERESTIMATE 0
#define OOEE_OVERESTIMATE 0
#define EEPP_OVERESTIMATE 0

MineManager::MineManager() {
  newRoutes = 0;
  maxX = -1;
  maxY = -1; // initted in prepareRoutes
}

MineManager::~MineManager() {
  for (set<MiningRoute*>::iterator it=routes.begin();
       it != routes.end();
       it++) { delete *it; }
  for (list<StationInfo*>::iterator it=allStations.begin();
       it != allStations.end();
       it++) { delete *it; }
  for (list<MiningRoute*>::iterator it=invalidRoutes.begin();
       it != invalidRoutes.end();
       it++) { delete *it; }
  for (list<CCenterInfo*>::iterator it=cCenters.begin();
       it != cCenters.end();
       it++) { delete *it; }
  for (set<MineralInfo*>::iterator it=minerals.begin();
       it != minerals.end();
       it++) { delete *it; }
}

void MineManager::prepareRoutes(list<SoarGameObject*>& miners) {
  maxX = Sorts::OrtsIO->getMapXDim();
  maxY = Sorts::OrtsIO->getMapYDim();
  list<MiningRoute*> bestRoutes;
  int numAssignments = miners.size();
  MiningRoute* best;

  msg << "preparing " << numAssignments << " routes.\n";

  // if this fails, not every route that was prepared last time was requested 
  // by the FSM.
  assert(assignments.size() == 0);
  
  // true if the corresponding miner has a route
  vector<bool> assigned;

  for (int i=0; i<numAssignments; i++) {
    best = getBestRoute();
    if (best == NULL) {
      return;
    }
    assert (best->stage == PF_DIST); 
    bestRoutes.push_back(best);
    addCostToRoute(best);
    
    assigned.push_back(false);
  }
  double minDistance;
  int minMinerIdx;
  SoarGameObject* minMinerPtr;
  double currentDistance;

  list<SoarGameObject*>::iterator minerIt;
  for (list<MiningRoute*>::iterator it = bestRoutes.begin();
       it != bestRoutes.end();
       it++) {
    minDistance = 99999999;
    minMinerIdx = -1;
    minMinerPtr = NULL;
    minerIt = miners.begin();
    for (int i=0; i<numAssignments; i++) {
      if (not assigned[i]) {
        currentDistance = coordDistanceSq((*it)->miningLoc, 
                                          (*minerIt)->getLocation()); 
        if (currentDistance < minDistance) {
          minMinerIdx = i;
          minMinerPtr = *minerIt;
          minDistance = currentDistance;
        }
      }
      minerIt++;
    }
    assert(minerIt == miners.end());
    // we now have the closest miner to the given route.
    assigned[minMinerIdx] = true;
    assignments[minMinerPtr] = *it;
  }
  
}
MiningRoute* MineManager::getMiningRoute(MineFSM* fsm) {
  map<SoarGameObject*, MiningRoute*>::iterator route 
      = assignments.find(fsm->getSoarGameObject());
  if (route == assignments.end()) {
    // if prepareRoutes quit because there were no routes
    return NULL;
  }
  
  MiningRoute* ret = (*route).second;
  assignments.erase(route);
  addFSMToRoute(ret, fsm);
  return ret;
}

MiningRoute* MineManager::minerGivesUp(MiningRoute* route, MineFSM* fsm) {
  msg << "choosing new route.\n";
  if (newRoutes++ < MAX_NEW_ROUTES) {
    route->mineStation->optimality += UNUSABLE_OPTIMALITY;
    removeFromRoute(route, fsm);
    adjustOptimality(route);
    MiningRoute* best = getBestRoute();
    if (best != NULL) {
      addCostToRoute(best);
      addFSMToRoute(best, fsm);
    }
    return best;
  }
  else {
    msg << "no new route given out, MAX_NEW_ROUTES reached.\n";
    return NULL;
  }
}
void MineManager::addMineral(SoarGameObject* mineral) {
  MineralInfo* mi = new MineralInfo;
  mi->mineral = mineral;
  mi->stationsValid[0] = false;
  mi->stationsValid[1] = false;
  mi->stationsValid[2] = false;
  mi->stationsValid[3] = false;

  coordinate c = mineral->getLocation();
  msg << "adding mineral " << (int)mineral << " at " 
      << c.x << "," << c.y << endl;
  
  for (list<CCenterInfo*>::iterator it = cCenters.begin();
       it != cCenters.end();
       it++) {
    addRoute(*it, mi);
  }
    
  minerals.insert(mi);
}
void MineManager::addControlCenter(SoarGameObject* center) {
  CCenterInfo* cci = new CCenterInfo;
  cci->cCenter = center;
  cci->stationsValid[0] = false;
  cci->stationsValid[1] = false;
  cci->stationsValid[2] = false;
  cci->stationsValid[3] = false;
  
  for (set<MineralInfo*>::iterator it = minerals.begin();
       it != minerals.end();
       it++) {
    // can't iterate and modify at the same time..
    addRoute(cci, *it);
  }
  cCenters.push_back(cci);
}

void MineManager::removeMineral(SoarGameObject* mineral) {
  // if anyone is mining this mineral, we need to interrupt them
  // and re-route
  
  msg << "removing mineral " << (int)mineral << endl; 

  MineralInfo dummy;
  dummy.mineral = mineral;

  SoarGameObject* sgo;
  
  set<MineralInfo*>::iterator it = minerals.find(&dummy);
  assert(it != minerals.end());
  MineralInfo* mi = *it;
 
  list <SoarGameObject*> miners; // need to prepare routes for miners
  list<MiningRoute*>::iterator it2;
  list<MineFSM*>::iterator it3;
  list<MineFSM*> fsmsToAbort;
  
  for(it2 = mi->routes.begin();
      it2 != mi->routes.end();
      it2++) {
    if ((*it2)->valid) {
      (*it2)->valid = false;
      for (it3 = (*it2)->fsms.begin();
          it3 != (*it2)->fsms.end();
          it3++) {
        sgo = (*it3)->getSoarGameObject();
        miners.push_back(sgo);
        fsmsToAbort.push_back(*it3);

        // this frees the resources (dropoff point matters, in this case)
        removeFromRouteNoErase(*it2, it3);
      }
      invalidRoutes.push_back(*it2);
      // route will still appear at the other end
      routes.erase(*it2);
    } 
  }
  // delete the mineral
  minerals.erase(it);
  // don't worry about de-allocating the stations, that is all done in the
  // allStations list.. 
  prepareRoutes(miners);
  
  for (it3 = fsmsToAbort.begin();
       it3 != fsmsToAbort.end();
       it3++) {
    (*it3)->abortMining();
    // FSM will request the new route calculated above
  }
}    

void MineManager::removeControlCenter(SoarGameObject* center) {
  // if anyone is using this, we need to interrupt them
  list<SoarGameObject*> miners; // need to prepare routes for miners
  list<MineFSM*> fsmsToAbort;
  list<MineFSM*>::iterator it3;
  SoarGameObject* sgo;
  
  for (list<CCenterInfo*>::iterator it = cCenters.begin();
       it != cCenters.end();
       it++) {
    if ((*it)->cCenter == center) {
      for(list<MiningRoute*>::iterator it2 = (*it)->routes.begin();
          it2 != (*it)->routes.end();
          it2++) {
        if ((*it2)->valid) {
          for (it3 = (*it2)->fsms.begin();
              it3 != (*it2)->fsms.end();
              it3++) {
            sgo = (*it3)->getSoarGameObject();
            miners.push_back(sgo);
            fsmsToAbort.push_back(*it3);
            // this frees the resources (dropoff point matters, in this case)
            // and deletes the MineFSM from the route
            removeFromRouteNoErase(*it2, it3);
          }
          (*it2)->fsms.clear();
          // remove the route 
          (*it2)->valid = false;
          invalidRoutes.push_back(*it2);
          routes.erase(*it2);
        }
      }
    }
    // remove cCenter
    cCenters.erase(it);
    it = cCenters.end();
  }
  
  prepareRoutes(miners);
  
  for (it3 = fsmsToAbort.begin();
       it3 != fsmsToAbort.end();
       it3++) {
    (*it3)->abortMining();
    // FSM will request the new route calculated above
  }
}

void MineManager::removeWorker(MiningRoute* route, MineFSM* fsm) {
  removeFromRoute(route, fsm);
}

void MineManager::removeFromRoute(MiningRoute* route, MineFSM* fsm) {
  for (list<MineFSM*>::iterator it= route->fsms.begin();
       it != route->fsms.end();
       it++) {
    if (*it == fsm) {
      removeFromRouteNoErase(route, it);
    }
    route->fsms.erase(it);
    it = route->fsms.end();
  }
}

void MineManager::removeFromRouteNoErase(MiningRoute* route, 
                                  list<MineFSM*>::iterator fsmIt) {
  // added due to the stupid STL list erase problem
  route->mineStation->optimality -= MINE_COST;
#ifndef NO_DOS
  route->dropoffStation->optimality -= DROPOFF_COST;
#endif
  
  for (list<MiningRoute*>::iterator it = route->mineStation->routes.begin();
       it != route->mineStation->routes.end();
       it++) {
    if ((*it)->valid) {
      adjustOptimality(*it);
    }
  }
  for (list<MiningRoute*>::iterator it 
      = route->dropoffStation->routes.begin();
       it != route->dropoffStation->routes.end();
       it++) {
    if ((*it)->valid) {
      adjustOptimality(*it);
    }
  }
}
    
void MineManager::addFSMToRoute(MiningRoute* route, MineFSM* fsm) {
  // the cost must have been added!
  route->fsms.push_back(fsm);
}

void MineManager::addCostToRoute(MiningRoute* route) {
  assert(route->mineStation != NULL);
  assert(route->dropoffStation != NULL);
  route->mineStation->optimality += MINE_COST;
#ifndef NO_DOS
  route->dropoffStation->optimality += DROPOFF_COST;
#endif
  dbg << "adding costs to: " << (int) route->mineStation << " and " 
                             << (int) route->dropoffStation << endl;

  for (list<MiningRoute*>::iterator it = route->mineStation->routes.begin();
       it != route->mineStation->routes.end();
       it++) {
    adjustOptimality(*it);
  }
#ifndef NO_DOS
  for (list<MiningRoute*>::iterator it = route->dropoffStation->routes.begin();
       it != route->dropoffStation->routes.end();
       it++) {
    adjustOptimality(*it);
  }
#endif

  Sorts::spatialDB->addImaginaryWorker(route->miningLoc);
  Sorts::spatialDB->addImaginaryWorker(route->dropoffLoc);
}

void MineManager::adjustOptimality(MiningRoute* route) {
  // if a mineral disappears, there will be invalid routes around
  // they are not deleted, but not in routes anymore
  if (route->valid) {
    set<MiningRoute*>::iterator it = routes.find(route);
    assert(it != routes.end());
    routes.erase(it);
    calculateOptimality(route);
    routes.insert(route);
  }
}

void MineManager::calculateOptimality(MiningRoute* route) {
  // for now, just make it infeasible to use the same station as another route
  if (route->mineStation != NULL) {
#ifndef NO_DOS
    route->optimality = route->pathlength 
                        + 100 * (route->mineStation->optimality)
                        + 100 * (route->dropoffStation->optimality);
#else
    route->optimality = route->pathlength
                        + 100 * (route->mineStation->optimality);
#endif
  }
  else {
    route->optimality = route->pathlength;
  }
}

void MineManager::addRoute(CCenterInfo* cci, MineralInfo* mi) {
  coordinate miningLoc = mi->mineral->getLocation();
  coordinate ccLoc = cci->cCenter->getLocation();
  MiningRoute* route;
 
  // make a new route for each set of mining point / dropoff point pairs
  route = new MiningRoute;
  route->mineralInfo = mi;
  route->cCenterInfo = cci;
  route->miningLoc = miningLoc;
  route->dropoffLoc = ccLoc;
  route->valid = true;
  
  route->pathlength = coordDistance(route->miningLoc, route->dropoffLoc)
                     - CCENTER_MAXRADIUS
                     - MINERAL_RADIUS; 
  route->stage = STRAIGHT_LINE_DIST;
  route->mineStation = NULL;
  route->dropoffStation = NULL;
  calculateOptimality(route);
  routes.insert(route);
  mi->routes.push_back(route);
  cci->routes.push_back(route);
  msg << "added SLD route: " << (int)cci->cCenter << " to " << (int)mi->mineral << " opt: " << route->optimality <<  endl;
  msg << route->miningLoc.x << "," << route->miningLoc.y 
    << " -> " << route->dropoffLoc.x << "," << route->dropoffLoc.y << endl;
} 

MiningRoute* MineManager::getBestRoute() {
  // keep expanding routes until the best (top of the set)
  // is a point-to-point path
  MiningRoute* topRoute = *routes.begin();
  if (routes.begin() == routes.end()) {
    msg << "error: trying to mine w/o any routes (is at least one mineral and cc in view?\n";
    return NULL;
  }
  
  RouteHeuristicStage topStage = topRoute->stage;
  MiningRoute* newTop;
  bool earlyQuit = false;
  while (topStage != PF_DIST) {
    dbg << "looking at route "; 
    dbg << "opt: " << topRoute->optimality << endl;
    dbg << "stage: " << topStage << endl;
    switch (topStage) {
      case STRAIGHT_LINE_DIST:
        expandSLD(topRoute);
        break;
      case OBJ_OBJ_PF_DIST:
        // expand to 16 edge-edge routes
        expandObjObj(topRoute); 
        break;
      case EDGE_EDGE_PF_DIST:
        // expand to point-to-point routes
        expandEdgeEdge(topRoute);
        break;
      case STATION_IN_USE:
        expandSIU(topRoute);
        break;
      case PF_DIST:
        // this will never happen (while condition)
        break;
    
    }

    newTop = *routes.begin();
    if (routes.begin() == routes.end()) {
      return NULL;
    }
    assert(newTop->valid);
    
    topRoute = newTop;
    topStage = topRoute->stage;
    
  }

  msg << "top route is "
      << topRoute->dropoffLoc.x << "," << topRoute->dropoffLoc.y << " to "
      << topRoute->miningLoc.x << "," << topRoute->miningLoc.y << endl;
  msg << "top route optimality: " << topRoute->optimality << endl;

  return topRoute;
}

void MineManager::expandSLD(MiningRoute* route) {
  dbg << "expandSLD\n";
  double oldPath = route->pathlength;
  double oldOpt = route->optimality;

  route->pathlength = pathFindDist(route->mineralInfo->mineral,
                                   route->cCenterInfo->cCenter);
  assert (route->pathlength >= oldPath-1);
  route->stage = OBJ_OBJ_PF_DIST;
  if (route->pathlength == -1) {
    route->valid = false;
    routes.erase(route);
    invalidRoutes.push_back(route);
  }
  else {
    route->pathlength -= CCENTER_MAXRADIUS;
    route->pathlength -= MINERAL_RADIUS;
    route->pathlength += SLDOO_OVERESTIMATE;
    adjustOptimality(route);
    dbg << "opt is now: " << route->optimality << endl;
    dbg << "SLDOO opch " << (route->optimality - oldOpt) << endl;
    dbg << "for mineral " << ((int)(route->mineralInfo->mineral)) << endl;
  }
}
void MineManager::expandObjObj(MiningRoute* route) {
  dbg << "expandOO\n";
  dbg << "for mineral " << (int)route->mineralInfo->mineral << endl;
  // make 16 edge-edge routes from an obj-obj route
  // the existing locations in the route are the centers of the objects
  double oldOpt = route->optimality;
  
  MiningRoute* newRoute;
  // temporary stations for this expansion stage
  StationInfo* dropoffStation;
  StationInfo* miningStation;
  Direction mineralDirection;
  Direction cCenterDirection;
  
#ifndef NO_DOS
  for (int i=0; i<4; i++) {
    for (int j=0; j<4; j++) {
      newRoute = new MiningRoute(*route);
      switch (i) {
        case 0:
          // need to be w/in 2 to mine..
          mineralDirection = WEST;
          newRoute->miningLoc.x -= (WORKER_RADIUS + MINERAL_RADIUS + 1);
          break;
        case 1:
          mineralDirection = EAST;
          newRoute->miningLoc.x += (WORKER_RADIUS + MINERAL_RADIUS + 1);
          break;
        case 2:
          mineralDirection = NORTH;
          newRoute->miningLoc.y -= (WORKER_RADIUS + MINERAL_RADIUS + 1);
          break;
        case 3:
          mineralDirection = SOUTH;
          newRoute->miningLoc.y += (WORKER_RADIUS + MINERAL_RADIUS + 1);
          break;
      }
      switch (j) {
        case 0:
          // need to be w/in 3 to dropoff..
          cCenterDirection = WEST;
          newRoute->dropoffLoc.x -= (WORKER_RADIUS + CCENTER_MINRADIUS + 2);
          break;
        case 1:
          cCenterDirection = EAST;
          newRoute->dropoffLoc.x += (WORKER_RADIUS + CCENTER_MINRADIUS + 2);
          break;
        case 2:
          cCenterDirection = NORTH;
          newRoute->dropoffLoc.y -= (WORKER_RADIUS + CCENTER_MINRADIUS + 2);
          break;
        case 3:
          cCenterDirection = SOUTH;
          newRoute->dropoffLoc.y += (WORKER_RADIUS + CCENTER_MINRADIUS + 2);
          break;
      }
      if (newRoute->miningLoc.x > 0 &&
          newRoute->miningLoc.y > 0 &&
          newRoute->dropoffLoc.x > 0 &&
          newRoute->dropoffLoc.y > 0 &&
          newRoute->miningLoc.x < maxX &&
          newRoute->miningLoc.y < maxY &&
          newRoute->dropoffLoc.x < maxX &&
          newRoute->dropoffLoc.y < maxY) {
        newRoute->pathlength = pathFindDist(newRoute->miningLoc,
                                            newRoute->dropoffLoc,
                                            false);
      }
      else {
        newRoute->pathlength = -1;
      }
      
      if (newRoute->pathlength == -1) {
        dbg << "deleting unreachable edge-edge route\n";
        dbg << "from " << newRoute->dropoffLoc << " on " 
            << (int)newRoute->cCenterInfo->cCenter << " to " 
            << newRoute->miningLoc << " on " 
            << (int)newRoute->mineralInfo->mineral << endl;
        delete newRoute;
      }
      else {
        newRoute->pathlength += OOEE_OVERESTIMATE;
        calculateOptimality(newRoute);
        dbg << "OOEE opch " << (newRoute->optimality - oldOpt) << endl;
        dbg << "adding route: " << newRoute->optimality << endl;
        routes.insert(newRoute);
        newRoute->mineralInfo->routes.push_back(newRoute);
        newRoute->cCenterInfo->routes.push_back(newRoute);
        newRoute->stage = EDGE_EDGE_PF_DIST;
      }
    }
  }
#else // no dropoff stations
  for (int i=0; i<4; i++) {
    newRoute = new MiningRoute(*route);
    switch (i) {
      case 0:
        // need to be w/in 2 to mine..
        mineralDirection = WEST;
        newRoute->miningLoc.x -= (WORKER_RADIUS + MINERAL_RADIUS + 1);
        break;
      case 1:
        mineralDirection = EAST;
        newRoute->miningLoc.x += (WORKER_RADIUS + MINERAL_RADIUS + 1);
        break;
      case 2:
        mineralDirection = NORTH;
        newRoute->miningLoc.y -= (WORKER_RADIUS + MINERAL_RADIUS + 1);
        break;
      case 3:
        mineralDirection = SOUTH;
        newRoute->miningLoc.y += (WORKER_RADIUS + MINERAL_RADIUS + 1);
        break;
    }
    if (newRoute->miningLoc.x > 0 &&
        newRoute->miningLoc.y > 0 &&
        newRoute->miningLoc.x < maxX &&
        newRoute->miningLoc.y < maxY) {
      newRoute->pathlength = pathFindDist(newRoute->miningLoc,
                                          newRoute->dropoffLoc,
                                          false);
                            // - CCENTER_MINRADIUS - MINERAL_RADIUS;
    }
    else {
      newRoute->pathlength = -1;
    }
    
    if (newRoute->pathlength == -1) {
      dbg << "deleting unreachable edge-edge route\n";
      dbg << "from " << newRoute->dropoffLoc << " on " 
          << (int)newRoute->cCenterInfo->cCenter << " to " 
          << newRoute->miningLoc << " on " 
          << (int)newRoute->mineralInfo->mineral << endl;
      delete newRoute;
    }
    else {
      newRoute->pathlength += OOEE_OVERESTIMATE;
      calculateOptimality(newRoute);
      dbg << "OOEE opch " << (newRoute->optimality - oldOpt) << endl;
      dbg << "adding route: " << newRoute->optimality << endl;
      routes.insert(newRoute);
      newRoute->mineralInfo->routes.push_back(newRoute);
      newRoute->cCenterInfo->routes.push_back(newRoute);
      newRoute->stage = EDGE_EDGE_PF_DIST;
    }
  }
#endif
  // remove the original route
  routes.erase(route);
  route->valid = false;
  invalidRoutes.push_back(route);
}

#ifndef NO_DOS
void MineManager::expandEdgeEdge(MiningRoute* route) {
  dbg << "expandEE\n";

  double oldOpt = route->optimality;
  
  double pfCache[CC_EDGE_STATIONS/2 + 1];
  for (int i=0; i<(CC_EDGE_STATIONS/2 +1); i++) {pfCache[i]=-1;}  
  // make 22 point-point routes from an edge-edge route
  // the existing locations in the route are the centers of the edges
  
  Direction mineralDir 
    = getRelDirection(route->mineralInfo->mineral->getLocation(), 
                      route->miningLoc);
  Direction cCenterDir 
    = getRelDirection(route->cCenterInfo->cCenter->getLocation(), 
                      route->dropoffLoc);
    
  bool ccHasStations = route->cCenterInfo->stationsValid[cCenterDir];
  bool mineralHasStations = route->mineralInfo->stationsValid[mineralDir];
 
  MiningRoute* newRoute;
  
  if (not mineralHasStations) {
    allocateMiningStations(route->mineralInfo, mineralDir);
  }
  if (not ccHasStations) {
    allocateDropoffStations(route->cCenterInfo, cCenterDir);
  }
  
  StationInfo* mineStation;
  StationInfo* dropoffStation;
  // yes, we do initialize these before using (ignore the compiler warning)
  
  
  for (int i=0; i<MINERAL_EDGE_STATIONS; i++) {
    for (int j=0; j<CC_EDGE_STATIONS; j++) {
      switch (mineralDir) {
        case NORTH:
          mineStation = route->mineralInfo->northStations[i];
          break;
        case SOUTH:
          mineStation = route->mineralInfo->southStations[i];
          break;
        case EAST:
          mineStation = route->mineralInfo->eastStations[i];
          break;
        case WEST:
          mineStation = route->mineralInfo->westStations[i];
          break;
      }
      switch (cCenterDir) {
        case NORTH:
          dropoffStation = route->cCenterInfo->northStations[j];
          break;
        case SOUTH:
          dropoffStation = route->cCenterInfo->southStations[j];
          break;
        case EAST:
          dropoffStation = route->cCenterInfo->eastStations[j];
          break;
        case WEST:
          dropoffStation = route->cCenterInfo->westStations[j];
          break;
      }

      newRoute = new MiningRoute(*route);
      newRoute->mineStation = mineStation;
      newRoute->dropoffStation = dropoffStation;
      newRoute->stage = PF_DIST;

      // yes, this is redundant, 
      // (route->location == route->station->location)
      // but the location at the top level must exist, since some routes
      // don't have stations (as they are not fully expanded)
      newRoute->miningLoc = mineStation->location;
      newRoute->dropoffLoc = dropoffStation->location;

      if (dropoffStation->optimality >= UNUSABLE_OPTIMALITY or
          mineStation->optimality >= UNUSABLE_OPTIMALITY) {
        dbg << "rejecting EE route, station is full.\n";
        newRoute->stage = STATION_IN_USE; 
        newRoute->pathlength = route->pathlength;
        mineStation->routes.push_back(newRoute);
        dropoffStation->routes.push_back(newRoute);
        newRoute->mineralInfo->routes.push_back(newRoute);
        newRoute->cCenterInfo->routes.push_back(newRoute);
        
        calculateOptimality(newRoute);
        dbg << "adding SIU route: " << newRoute->optimality << endl;
        dbg << dropoffStation->location.x << "," << dropoffStation->location.y
            << "->" 
            << mineStation->location.x << "," << mineStation->location.y 
            << endl;
        dbg << "mineral: " << (int(newRoute->mineralInfo->mineral)) << endl;
        routes.insert(newRoute);
        
      }
      else if (collision(mineStation)) {
        dbg << "immediately rejecting EE route: collision.\n";
        delete newRoute;
      }
      else if (collision(dropoffStation)) {
        dbg << "immediately rejecting EE route: collision.\n";
        delete newRoute;
      }
      else {
#ifdef PF_CACHE
        if (i == 0 and (j%2 == 0)) {
          newRoute->pathlength 
            = pathFindDist(newRoute->dropoffStation->location, 
                           newRoute->mineStation->location,
                           true);
          pfCache[j/2] = newRoute->pathlength;
        }
        else {
          if (pfCache[j/2] != -1) {
            newRoute->pathlength = pfCache[j/2];
          }
          else {
            // we skipped the station that was supposed to set this
            newRoute->pathlength 
              = pathFindDist(newRoute->dropoffStation->location, 
                            newRoute->mineStation->location,
                            true);
            pfCache[j/2] = newRoute->pathlength;
          }
            
        }
#else
        newRoute->pathlength 
          = pathFindDist(newRoute->dropoffStation->location, 
                         newRoute->mineStation->location,
                         true);
#endif  

        if (newRoute->pathlength == -1) {
          dbg << "deleting unreachable point-point route\n";
          delete newRoute;
        }
        else {
          // register the route with everyone who needs to know about it
          mineStation->routes.push_back(newRoute);
          dropoffStation->routes.push_back(newRoute);
          newRoute->mineralInfo->routes.push_back(newRoute);
          newRoute->cCenterInfo->routes.push_back(newRoute);
          newRoute->pathlength += EEPP_OVERESTIMATE;
          
          calculateOptimality(newRoute);
          dbg << "adding route: " << newRoute->optimality << endl;
          dbg << "EEPP opch " << (newRoute->optimality - oldOpt) << endl;
          dbg << dropoffStation->location.x << "," << dropoffStation->location.y
              << "->" 
              << mineStation->location.x << "," << mineStation->location.y 
              << endl;
          dbg << "mineral: " << (int(newRoute->mineralInfo->mineral)) << endl;
          routes.insert(newRoute);
        }
      }
    }
  }

  route->valid = false;
  routes.erase(route);
  invalidRoutes.push_back(route);
}
# else // no dropoff stations
void MineManager::expandEdgeEdge(MiningRoute* route) {
  dbg << "expandEE\n";

  double oldOpt = route->optimality;
  double pfCache = -1;
  
  Direction mineralDir 
    = getRelDirection(route->mineralInfo->mineral->getLocation(), 
                      route->miningLoc);

  bool mineralHasStations = route->mineralInfo->stationsValid[mineralDir];
 
  MiningRoute* newRoute;
  
  if (not mineralHasStations) {
    allocateMiningStations(route->mineralInfo, mineralDir);
  }
  
  StationInfo* mineStation;
  StationInfo* dropoffStation = new StationInfo;

  dropoffStation->location = route->cCenterInfo->cCenter->getLocation();
  
  for (int i=0; i<MINERAL_EDGE_STATIONS; i++) {
    switch (mineralDir) {
      case NORTH:
        mineStation = route->mineralInfo->northStations[i];
        break;
      case SOUTH:
        mineStation = route->mineralInfo->southStations[i];
        break;
      case EAST:
        mineStation = route->mineralInfo->eastStations[i];
        break;
      case WEST:
        mineStation = route->mineralInfo->westStations[i];
        break;
    }
    newRoute = new MiningRoute(*route);
    newRoute->mineStation = mineStation;
    newRoute->dropoffStation = dropoffStation;
    newRoute->stage = PF_DIST;

    // yes, this is redundant, 
    // (route->location == route->station->location)
    // but the location at the top level must exist, since some routes
    // don't have stations (as they are not fully expanded)
    newRoute->miningLoc = mineStation->location;

    if (mineStation->optimality >= UNUSABLE_OPTIMALITY) {
      dbg << "rejecting EE route, station is full.\n";
      newRoute->stage = STATION_IN_USE; 
      newRoute->pathlength = route->pathlength;
      mineStation->routes.push_back(newRoute);
      newRoute->mineralInfo->routes.push_back(newRoute);
      newRoute->cCenterInfo->routes.push_back(newRoute);
      
      calculateOptimality(newRoute);
      dbg << "adding SIU route: " << newRoute->optimality << endl;
      dbg << "mineral: " << (int(newRoute->mineralInfo->mineral)) << endl;
      routes.insert(newRoute);
      
    }
    else if (collision(mineStation)) {
      dbg << "immediately rejecting EE route: collision.\n";
      delete newRoute;
    }
    else {
      if (i == 0) {
        newRoute->pathlength 
          = pathFindDist(newRoute->dropoffStation->location, 
                          newRoute->mineStation->location,
                          true);
        pfCache = newRoute->pathlength;
      }
      else {
        if (pfCache != -1) {
          newRoute->pathlength = pfCache;
        }
        else {
          // we skipped the station that was supposed to set this
          newRoute->pathlength 
            = pathFindDist(newRoute->dropoffStation->location, 
                          newRoute->mineStation->location,
                          true);
          pfCache = newRoute->pathlength;
        }
          
      }

      if (newRoute->pathlength == -1) {
        dbg << "deleting unreachable point-point route\n";
        delete newRoute;
      }
      else {
        // register the route with everyone who needs to know about it
        mineStation->routes.push_back(newRoute);
        newRoute->mineralInfo->routes.push_back(newRoute);
        newRoute->cCenterInfo->routes.push_back(newRoute);
        newRoute->pathlength += EEPP_OVERESTIMATE;
        
        calculateOptimality(newRoute);
        dbg << "adding route: " << newRoute->optimality << endl;
        dbg << "EEPP opch " << (newRoute->optimality - oldOpt) << endl;
        dbg << "mineral: " << (int(newRoute->mineralInfo->mineral)) << endl;
        routes.insert(newRoute);
      }
    }
  }

  route->valid = false;
  routes.erase(route);
  invalidRoutes.push_back(route);
}
#endif

void MineManager::expandSIU(MiningRoute* route) {
  route->pathlength 
     = pathFindDist(route->dropoffStation->location, 
                    route->mineStation->location,
                    true);
  route->stage = PF_DIST;
  if (route->pathlength == -1) {
    dbg << "deleting unreachable point-point route\n";
    routes.erase(route);
    route->valid = false;
  }
  else {
    routes.erase(route);
    calculateOptimality(route);
    dbg << "route back from the dead: " << route->optimality << endl;
    dbg << "mineral: " << (int(route->mineralInfo->mineral)) << endl;
    routes.insert(route);
  }
} 

void MineManager::allocateMiningStations(MineralInfo* mi, Direction d) {
  StationInfo* newStation;
  coordinate mineralCenter = mi->mineral->getLocation();
  
  assert (MINERAL_EDGE_STATIONS == 2);
  for (int i=0; i<MINERAL_EDGE_STATIONS; i++) {
    newStation = new StationInfo;
    allStations.push_back(newStation);
    switch (d) {
      case SOUTH:
        mi->southStations[i] = newStation;
        newStation->location.x = mineralCenter.x - 4 + i*7;
        newStation->location.y 
          = mineralCenter.y + WORKER_RADIUS + MINERAL_RADIUS + 1;
        break;
      case NORTH:
        mi->northStations[i] = newStation;
        newStation->location.x = mineralCenter.x - 4 + i*7;
        newStation->location.y 
          = mineralCenter.y - (WORKER_RADIUS + MINERAL_RADIUS + 1);
        break;
      case EAST:
        mi->eastStations[i] = newStation;
        newStation->location.x 
          = mineralCenter.x + WORKER_RADIUS + MINERAL_RADIUS + 1;
        newStation->location.y = mineralCenter.y - 4 + i*7;
        break;
      case WEST:
        mi->westStations[i] = newStation;
        newStation->location.x 
          = mineralCenter.x - (WORKER_RADIUS + MINERAL_RADIUS + 1);
        newStation->location.y = mineralCenter.y - 4 + i*7;
        break;
    }
    newStation->optimality = 0;
  }
  mi->stationsValid[d] = true;
}

void MineManager::allocateDropoffStations(CCenterInfo* cci, Direction d) {
  StationInfo* newStation;
  coordinate ccCenter = cci->cCenter->getLocation();
  
  assert (CC_EDGE_STATIONS == 10);
  for (int i=0; i<10; i++) {
    newStation = new StationInfo;
    switch (d) {
      case SOUTH:
        cci->southStations[i] = newStation;
        newStation->location.x =(int)(ccCenter.x - 32 + i*7);
        newStation->location.y 
          = ccCenter.y + WORKER_RADIUS + CCENTER_MINRADIUS + 2;
        break;
      case NORTH:
        cci->northStations[i] = newStation;
        newStation->location.x =(int)( ccCenter.x - 32 + i*7);
        newStation->location.y 
          = ccCenter.y - (WORKER_RADIUS + CCENTER_MINRADIUS + 2);
        break;
      case EAST:
        cci->eastStations[i] = newStation;
        newStation->location.x 
          = ccCenter.x + WORKER_RADIUS + CCENTER_MINRADIUS + 2;
        newStation->location.y =(int)( ccCenter.y - 32 + i*7);
        break;
      case WEST:
        cci->westStations[i] = newStation;
        newStation->location.x 
          = ccCenter.x - (WORKER_RADIUS + CCENTER_MINRADIUS + 2);
        newStation->location.y =(int)( ccCenter.y - 32 + i*7);
        break;
    }
    newStation->optimality = 0;
  }
  cci->stationsValid[d] = true;
}

double MineManager::pathFindDist(SoarGameObject* obj1, SoarGameObject* obj2) {
  SortsSimpleTerrain::Path path;
  SortsSimpleTerrain::Loc prevLoc;
  dbg << "finding path:\n"
      << *(obj1->getGob()->sod.x) << ","<< *(obj1->getGob()->sod.y)
      << " -> " << *(obj2->getGob()->sod.x) << ","<< *(obj2->getGob()->sod.y)
      << endl;
  Sorts::terrainModule->findPath(obj1->getGob(), obj2->getGob(), path);
  double result = 0;
  if (path.locs.size() == 0) {
    result = -1;
  }
  else {
    // path is obj1->obj2, waypoints in reverse order
    prevLoc.x = *obj1->getGob()->sod.x;
    prevLoc.y = *obj1->getGob()->sod.y;
    dbg << "loc: " << prevLoc.x << "," << prevLoc.y << endl;
    for (int i=path.locs.size()-1; i>=0; i--) {
      result += path.locs[i].distance(prevLoc);
      dbg << "loc: " << path.locs[i].x << "," << path.locs[i].y << endl;
      prevLoc = path.locs[i];
    }
    dbg << "then: " <<*obj2->getGob()->sod.x << "," << *obj2->getGob()->sod.y << endl;
  }
  dbg << "raw pf dist: " << result << "\n";
  return result;
}

double MineManager::pathFindDist(coordinate loc1, coordinate loc2, 
                                 bool penalizeWaypoints) {
  SortsSimpleTerrain::Path path;
  SortsSimpleTerrain::Loc prevLoc;
  SortsSimpleTerrain::Loc tloc1;
  SortsSimpleTerrain::Loc tloc2;
  tloc1.x = loc1.x;
  tloc1.y = loc1.y;
  tloc2.x = loc2.x;
  tloc2.y = loc2.y;
  Sorts::terrainModule->findPath(tloc1, tloc2, path);
  double result = 0;
  if (path.locs.size() == 0) {
    result = -1;
  }
  else {
    // path is obj1->obj2, waypoints in reverse order
    prevLoc.x = loc1.x;
    prevLoc.y = loc1.y;
    //dbg << "loc: " << loc1.x << "," << loc1.y << endl;
    for (int i=path.locs.size()-1; i>=0; i--) {
      //dbg << "loc: " << path.locs[i].x << "," << path.locs[i].y << endl;
      result += path.locs[i].distance(prevLoc);
      if (penalizeWaypoints) {
        result += WAYPOINT_PENALTY;
      }
      prevLoc = path.locs[i];
    }
  //dbg << "accumulated " << (path.locs.size()-2)*WAYPOINT_PENALTY << " wpp\n";
  }
  dbg << "raw pf dist: " << result << "\n";
  dbg << "w/o radii " << result - CCENTER_MINRADIUS - MINERAL_RADIUS << endl;
  return result;
}

Direction MineManager::getRelDirection(coordinate first, coordinate second) {
  // return relative direction of second to first
  
  assert(second.x == first.x || second.y == first.y);
  if (second.x > first.x) {
    return EAST;
  }
  if (second.y > first.y) {
    return SOUTH;
  }
  if (second.x < first.x) {
    return WEST;
  }
  else {
    assert(second.y < first.y);
    return NORTH;
  }
}

bool MineManager::collision(StationInfo* station) {
  // return true if the station collides with an object other than a worker,
  // sheep, or controlCenter
  if (Sorts::spatialDB->hasMiningCollision(station->location)) {
    dbg << "setting high opt: collision at " << (int)station << endl;
    station->optimality = UNUSABLE_OPTIMALITY;
    return true;
  }
  return false;
}

