#include "MineManager.h"
#include "Sorts.h"
#include "MineFSM.h"

/*
   MineManager.cpp
   Sorts project
   Sam Wintermute, 2006
*/

MineManager::MineManager() {
  nextStationID = 0;
}

MineManager::~MineManager() {
  for (set<MiningRoute*>::iterator it=routes.begin();
       it != routes.end();
       it++) { delete *it; }
}

void MineManager::prepareRoutes(list<SoarGameObject*>& miners) {
  list<MiningRoute*> bestRoutes;
  int numAssignments = miners.size();
  MiningRoute* best;

  // if this fails, not every route that was prepared last time was requested 
  // by the FSM.
  assert(assignments.size() == 0);
  
  // true if the corresponding miner has a route
  vector<bool> assigned;

  for (int i=0; i<numAssignments; i++) {
    best = getBestRoute();
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
        currentDistance = coordDistanceSq((*it)->mineralLoc, 
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
  assert(route != assignments.end());
  MiningRoute* ret = (*route).second;
  assignments.erase(route);
  addFSMToRoute(ret, fsm);
  return ret;
}
MiningRoute* MineManager::reportMiningResults(int time, MiningRoute* route, 
                                              bool atBase, MineFSM* fsm) {
  return NULL;
}
void MineManager::addMineral(SoarGameObject* mineral) {
  MineralInfo mi;
  mi.mineral = mineral;
  
  for (list<cCenterInfo>::iterator it = cCenters.begin();
       it != cCenters.end();
       it++) {
    addRoute(&(*it), &mi);
  }
    
  minerals.insert(mi);
}
void MineManager::addControlCenter(SoarGameObject* center) {
  cCenterInfo cci;
  cci.cCenter = center;
  
  for (set<MineralInfo>::iterator it = minerals.begin();
       it != minerals.end();
       it++) {
    addRoute(&cci, &(*it));
  }
    
  cCenters.push_back(cci);
}

void MineManager::removeMineral(SoarGameObject* mineral) {
  // if anyone is mining this mineral, we need to interrupt them
  // and re-route

  MineralInfo dummy;
  dummy.mineral = mineral;

  SoarGameObject* sgo;
  
  set<MineralInfo>::iterator it = minerals.find(dummy);
  assert(it != minerals.end());
  MineralInfo mi = *it;
 
  list <SoarGameObject*> miners; // need to prepare routes for miners
  list<MiningRoute*>::iterator it2;
  list<MineFSM*>::iterator it3;
  list<MineFSM*> fsmsToAbort;
  
  for(it2 = mi.routes.begin();
      it2 != mi.routes.end();
      it2++) {
    for (it3 = (*it2)->fsms.begin();
         it3 != (*it2)->fsms.end();
         it3++) {
      sgo = (*it3)->getSoarGameObject();
      miners.push_back(sgo);
      fsmsToAbort.push_back(*it3);

      // this frees the resources (dropoff point matters, in this case)
      // and deletes the MineFSM from the route
      removeFromRoute(*it2, it3);
    }
    // delete the route
    routes.erase(*it2);
    delete *it2;
  }
  // delete the mineral
  minerals.erase(it);
  
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
  
  for (list<cCenterInfo>::iterator it = cCenters.begin();
       it != cCenters.end();
       it++) {
    if ((*it).cCenter == center) {
      for(list<MiningRoute*>::iterator it2 = (*it).routes.begin();
          it2 != (*it).routes.end();
          it2++) {
        for (it3 = (*it2)->fsms.begin();
             it3 != (*it2)->fsms.end();
             it3++) {
          sgo = (*it3)->getSoarGameObject();
          miners.push_back(sgo);
          fsmsToAbort.push_back(*it3);
          // this frees the resources (dropoff point matters, in this case)
          // and deletes the MineFSM from the route
          removeFromRoute(*it2, it3);
        }
        // remove the route 
        routes.erase(*it2);
        delete *it2;
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
      removeFromRoute(route, it);
    }
  }
}

void MineManager::removeFromRoute(MiningRoute* route, 
                                  list<MineFSM*>::iterator it) {
  //assignments.erase(*it->getSoarGameObject());
  route->fsms.erase(it);
  route->mineStation->optimality--;
  route->dropoffStation->optimality--;
  adjustOptimality(route);
}

    
void MineManager::addFSMToRoute(MiningRoute* route, MineFSM* fsm) {
  // the cost must have been added!
  route->fsms.push_back(fsm);
}

void MineManager::addCostToRoute(MiningRoute* route) {
  route->mineStation->optimality++;
  route->dropoffStation->optimality++;
  adjustOptimality(route);
}

void MineManager::adjustOptimality(MiningRoute* route) {
  set<MiningRoute*>::iterator it = routes.find(route);
  assert(it != routes.end());
  routes.erase(it);
  calculateOptimality(route);
  routes.insert(route);
}

void MineManager::calculateOptimality(MiningRoute* route) {
  // for now, just make it infeasible to use the same station as another route
  if (route->mineStation != NULL) {
    route->optimality = route->pathlength 
                        + 100 * (route->mineStation->optimality)
                        + 100 * (route->dropoffStation->optimality);
  }
  else {
    route->optimality = route->pathlength;
  }
}

void MineManager::addRoute(cCenterInfo* cci, const MineralInfo* mi) {
  coordinate mineralLoc = mi->mineral->getLocation();
  coordinate ccLoc = cci->cCenter->getLocation();
  MiningRoute* route;
 
  // make a new route for each set of mining point / dropoff point pairs
  route = new MiningRoute;
  route->mineral = mi->mineral;
  route->cCenter = cci->cCenter;
  route->mineralLoc = mineralLoc;
  route->dropoffLoc = ccLoc;
  
  route->pathlength = coordDistance(route->mineralLoc, route->dropoffLoc)
                     - CCENTER_MAXRADIUS
                     - MINERAL_RADIUS; 
  route->stage = STRAIGHT_LINE_DIST;
  route->mineStation = NULL;
  route->dropoffStation = NULL;
  routes.insert(route);
} 

MiningRoute* MineManager::getBestRoute() {
  // keep expanding routes until the best (top of the set)
  // is a point-to-point path
  MiningRoute* topRoute = *routes.begin();
  RouteHeuristicStage topStage = topRoute->stage;
  
  while (topStage != PF_DIST) {
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
      case PF_DIST:
        // this will never happen (while condition)
        break;
    
    }
    topRoute = *routes.begin();
    topStage = topRoute->stage;
  }

  return topRoute;
}

void MineManager::expandSLD(MiningRoute* route) {
  route->pathlength = pathFindDist(route->mineral, route->cCenter);
  route->stage = OBJ_OBJ_PF_DIST;
  if (route->pathlength == -1) {
    routes.erase(route);
    delete route;
  }
  else {
    route->pathlength -= CCENTER_MAXRADIUS;
    route->pathlength -= MINERAL_RADIUS;
    adjustOptimality(route);
  }
}
void MineManager::expandObjObj(MiningRoute* route) {
  // make 16 edge-edge routes from an obj-obj route
  // the existing locations in the route are the centers of the objects
  
  MiningRoute* newRoute;
  
  for (int i=0; i<4; i++) {
    for (int j=0; j<4; j++) {
      newRoute = new MiningRoute(*route);
      switch (i) {
        case 0:
          // need to be w/in 2 to mine..
          newRoute->mineralLoc.x -= (WORKER_RADIUS + MINERAL_RADIUS + 1);
          break;
        case 1:
          newRoute->mineralLoc.x += (WORKER_RADIUS + MINERAL_RADIUS + 1);
          break;
        case 2:
          newRoute->mineralLoc.y -= (WORKER_RADIUS + MINERAL_RADIUS + 1);
          break;
        case 3:
          newRoute->mineralLoc.y += (WORKER_RADIUS + MINERAL_RADIUS + 1);
          break;
      }
      switch (j) {
        case 0:
          // need to be w/in 3 to dropoff..
          newRoute->dropoffLoc.x -= (WORKER_RADIUS + CCENTER_MINRADIUS + 2);
          break;
        case 1:
          newRoute->dropoffLoc.x += (WORKER_RADIUS + CCENTER_MINRADIUS + 2);
          break;
        case 2:
          newRoute->dropoffLoc.y -= (WORKER_RADIUS + CCENTER_MINRADIUS + 2);
          break;
        case 3:
          newRoute->dropoffLoc.y += (WORKER_RADIUS + CCENTER_MINRADIUS + 2);
          break;
      }
    }
    newRoute->pathlength = pathFindDist(newRoute->dropoffLoc, 
                                        newRoute->mineralLoc);
    newRoute->stage = EDGE_EDGE_PF_DIST;
    if (newRoute->pathlength == -1) {
      cout << "deleting unreachable edge-edge route\n";
      delete newRoute;
    }
    else {
      calculateOptimality(newRoute);
      routes.insert(newRoute);
    }
  }
  // remove the original route
  routes.erase(route);
  delete route;
}

void MineManager::expandEdgeEdge(MiningRoute* route) {
  // make 22 point-point routes from an edge-edge route
  // the existing locations in the route are the centers of the edges

  // find out if the edges are vertical or horizontal- are we varying x or y as
  // routes are created?
  bool verticalOnCC = (*route->cCenter->gob->sod.x == route->dropoffLoc.x);
  bool verticalOnMineral = (*route->mineral->gob->sod.x == route->mineralLoc.x);
  
  MiningRoute* newRoute;
  
  for (int i=0; i<2; i++) {
    for (int j=0; j<11; j++) {
      newRoute = new MiningRoute(*route);
      if (verticalOnMineral) {
        switch (i) {
          case 0:
            newRoute->mineralLoc.y -= 3;
            break;
          case 1:
            newRoute->mineralLoc.y += 3;
            break;
        }
      }
      else {
        switch (i) {
          case 0:
            newRoute->mineralLoc.x -= 3;
            break;
          case 1:
            newRoute->mineralLoc.x += 3;
            break;
        }
      }
      // we can fit 11 workers on an edge of a cc
      if (verticalOnCC) {
        newRoute->dropoffLoc.x -= 15 + 3*j;
      }
      else {
        newRoute->dropoffLoc.x -= 15 + 3*j;
      }
    }
    newRoute->pathlength = pathFindDist(newRoute->dropoffLoc, 
                                        newRoute->mineralLoc);
    newRoute->stage = PF_DIST;
    if (newRoute->pathlength == -1) {
      cout << "deleting unreachable edge-edge route\n";
      delete newRoute;
    }
    else {
      calculateOptimality(newRoute);
      routes.insert(newRoute);
    }
  }
  // remove the original route
  routes.erase(route);
  delete route;
}

double MineManager::pathFindDist(SoarGameObject* obj1, SoarGameObject* obj2) {
  TerrainBase::Path path;
  TerrainBase::Loc prevLoc;
 // Sorts::terrainModule->findPath(obj1->gob, obj2->gob, path);
  double result = 0;
  if (path.locs.size() == 0) {
    result = -1;
  }
  else {
    // path is obj1->obj2, waypoints in reverse order
    prevLoc.x = *obj1->gob->sod.x;
    prevLoc.y = *obj1->gob->sod.y;
    for (int i=path.locs.size()-1; i>=0; i--) {
      result += path.locs[i].distance(prevLoc);
      prevLoc = path.locs[i];
    }
  }
  return result;
}

double MineManager::pathFindDist(coordinate loc1, coordinate loc2) {
  TerrainBase::Path path;
  TerrainBase::Loc prevLoc;
  TerrainBase::Loc tloc1;
  TerrainBase::Loc tloc2;
  tloc1.x = loc1.x;
  tloc1.y = loc1.y;
  tloc2.x = loc2.x;
  tloc2.y = loc2.y;
//  Sorts::terrainModule->findPath(tloc1, tloc2, path);
  double result = 0;
  if (path.locs.size() == 0) {
    result = -1;
  }
  else {
    // path is obj1->obj2, waypoints in reverse order
    prevLoc.x = loc1.x;
    prevLoc.y = loc1.y;
    for (int i=path.locs.size()-1; i>=0; i--) {
      result += path.locs[i].distance(prevLoc);
      prevLoc = path.locs[i];
    }
  }
  return result;
}
