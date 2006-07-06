#include <sys/time.h>
#include <assert.h>

// our includes
#include "Sorts.h"
#include "PerceptualGroup.h"
//#include "InternalGroup.h"
#include "MineManager.h"

// Orts includes
#include "GameObj.H"
#include "GameStateModule.H"

#define CLASS_TOKEN "ORTSIO"
#define DEBUG_OUTPUT false 
#include "OutputDefinitionsUnique.h"

#ifdef USE_CANVAS
#include "SortsCanvas.h"
#endif

#define NO_WORLD_GROUPS

// how many frames behind can we be?
// 10 used for competition,
// but what can we really do if we're behind? most actions are set every cycle
#define ALLOWED_LAG 0

OrtsInterface::OrtsInterface(GameStateModule* _gsm)
: gsm(_gsm)
{
  counter = 0;
  // gsm must have already connected for this to work
  myPid = gsm->get_game().get_client_player(); 
  mapXDim = (gsm->get_game().get_gtiles_x())*(gsm->get_game().get_tile_points());
  mapYDim = (gsm->get_game().get_gtiles_y())*(gsm->get_game().get_tile_points());
  lastActionFrame = 0;
  gold = 0;
  viewFrame = -1;
  skippedActions = 0;
  PlayerInfo &pi = gsm->get_game().get_cplayer_info();
  playerGameObj = (GameObj*)(pi.global_obj("player"));
  buildingThisCycle = false;
  lastError = 0;
  numWorkers = 0;
  numMarines = 0;
  numTanks = 0;
  unitCountsStale = false;
  reachabilityObject = NULL;
  deadCount = 0;
}

bool OrtsInterface::handle_event(const Event& e) {
  unsigned long st = gettime();
  pthread_mutex_lock(Sorts::mutex);
  if (e.get_who() == GameStateModule::FROM) {
    if (e.get_what() == GameStateModule::VIEW_MSG) {
      dbg << "SOARCYCLES: " << Sorts::cyclesSoarAhead << endl;
      Sorts::cyclesSoarAhead = 0;
      msg << "ORTS EVENT {\n";
      viewFrame = gsm->get_game().get_view_frame();
      Sorts::frame = viewFrame;
      if (Sorts::catchup) {
        // we haven't modified catchup yet, 
        // so this means the last event was skipped
        // accumulate skipped actions, in that case
        skippedActions += gsm->get_game().get_skipped_actions();
      }
      else {
        skippedActions = gsm->get_game().get_skipped_actions();
      }
      int aFrame = gsm->get_game().get_action_frame();
        
      mergeChanges(const_cast<GameChanges&> (gsm->get_changes())); 
      // cleared every event, so we need to merge the changes for skipped
      // events

      // if you can figure out how to iterate through a vector inside a
      // const'd class, feel free to fix the const_cast
      
      if (aFrame != -1) {
        lastActionFrame = aFrame;
      }
      msg << "frame beginning " 
           << viewFrame << "/" << lastActionFrame << endl;
      if ((viewFrame -lastActionFrame) < ALLOWED_LAG and Sorts::catchup) {
        msg << "caught up at frame " << viewFrame << endl;
      }
      if (skippedActions) {
        msg << "WARNING: skipped actions: " << skippedActions << endl;
      }
      if ((viewFrame - lastActionFrame) > ALLOWED_LAG) {
        Sorts::catchup = true;
        msg << "client is behind, skipping event. v: "
             << viewFrame << " a:" << lastActionFrame << "\n";
#ifdef USE_CANVAS
        Sorts::canvas.setStatus("skipping event");
#endif
        gsm->send_actions(); // empty, needed by server
        pthread_mutex_unlock(Sorts::mutex);
        Sorts::SoarIO->startSoar();
        dbg << "TIME " << (gettime() - st) / 1000 << endl; 
        return true;
      }
#ifdef USE_CANVAS
      stringstream ss;
      string status;
      ss << "ORTS Event " << viewFrame;
      if (skippedActions) {
        ss << " SKIPPED ACTIONS: " << skippedActions;
      }
      status = ss.str();
      Sorts::canvas.setStatus(status);
      Sorts::canvas.update();
#endif
      
      Sorts::SoarIO->updateViewFrame(viewFrame);
      Sorts::catchup = false;
      
      int merged = gsm->get_game().get_merged_actions();
      if (merged) {
        // these appear to be harmless
        //  cout << "WARNING: merged actions (what does this mean?)" << merged << endl;
      }

      removeDeadObjects();
      Sorts::pGroupManager->assignActions();
      updateMap();
      updateSoarGameObjects();
      changes.clear();


      // since the FSM's have been updated, we should send the actions here
      gsm->send_actions();

      Sorts::pGroupManager->updateGroups();

      /* I'm assuming here that those update calls from above have already
       * updated the soar input link correctly, so commit everything
       */
      Sorts::SoarIO->commitInputLinkChanges();
      Sorts::SoarIO->startSoar();
    }
    msg << "ORTS EVENT }" << endl;
#ifdef USE_CANVAS
    Sorts::canvas.clearStatus();
#endif
    pthread_mutex_unlock(Sorts::mutex);
    msg << "TIME " << (gettime() - st) / 1000 << endl; 
    return true;
  }
  else {
    pthread_mutex_unlock(Sorts::mutex);
    msg << "TIME " << (gettime() - st) / 1000 << endl; 
    return false;
  }
}

void OrtsInterface::addAppearedObject(const GameObj* gameObj) {
  assert(false);
}

void OrtsInterface::addCreatedObject(GameObj* gameObj) {
  // make sure the game object does not exist in the middleware
  assert(objectMap.find(gameObj) == objectMap.end());
 
  bool friendly = (myPid == *gameObj->sod.owner);
  bool world    = (gsm->get_game().get_player_num() == *gameObj->sod.owner);
  int id = gsm->get_game().get_cplayer_info().get_id(gameObj);
  
  string name = gameObj->bp_name();

  if (name == "start_loc") {
    msg << "ignoring start_loc GameObj.\n";
    return;
  }
  
  SoarGameObject* newObj = new SoarGameObject(gameObj,
                                              friendly, world, id);
 

#ifdef NO_WORLD_GROUPS
  if (not world) {
    Sorts::pGroupManager->makeNewGroup(newObj);
  }
#else 
  Sorts::pGroupManager->makeNewGroup(newObj);
#endif

  if (name == "mineral") {
    Sorts::mineManager->addMineral(newObj);
  }
  else if (friendly && name == "controlCenter") {
    Sorts::mineManager->addControlCenter(newObj);
  }
  else if (friendly && name == "worker") {
    numWorkers++;
    unitCountsStale = true;
  }
  else if (friendly && name == "marine") {
    numMarines++;
    unitCountsStale = true;
  }
  else if (friendly && name == "tank") {
    numTanks++;
    unitCountsStale = true;
  }

  if (friendly && (name == "controlCenter" ||
                   name == "barracks" ||
                   name == "factory")) {
    friendlyBuildings.push_back(gameObj);
    if (reachabilityObject == NULL) {
      reachabilityObject = gameObj;
    }
  }

#ifdef USE_CANVAS
  Sorts::canvas.registerSGO(newObj);
#endif
  
  
  msg << gameObj << " added, id " << id << endl;
  objectMap[gameObj] = newObj;
   
  if (liveIDs.find(id) != liveIDs.end()) {
    msg << "ERROR: appeared object is there already: " << gameObj << endl;
  }

  newObj->update();
  assert(liveIDs.find(id) == liveIDs.end());
  liveIDs.insert(id);
}

void OrtsInterface::removeDeadObject(const GameObj* gameObj) {
  // make sure the game object exists
  assert(objectMap.find(gameObj) != objectMap.end());
  deadCount++;

  SoarGameObject* sObject = objectMap[gameObj];
  bool friendly = sObject->isFriendly();
  string name = sObject->getName();
  int id = sObject->getID();
  if (sObject->getPerceptualGroup() != NULL) {
    sObject->getPerceptualGroup()->removeUnit(sObject);
  } 
  if (name == "mineral") {
    Sorts::mineManager->removeMineral(sObject);
  }
  else if (friendly and name == "controlCenter") {
    Sorts::mineManager->removeControlCenter(sObject);
  }
  
  if (friendly && (name == "controlCenter" ||
                   name == "barracks" ||
                   name == "factory")) {
    friendlyBuildings.remove(gameObj);
    if (reachabilityObject == gameObj) {
      if (friendlyBuildings.begin() != friendlyBuildings.end()) {
        reachabilityObject = const_cast<GameObj*> (*friendlyBuildings.begin());
      }
      else {
        reachabilityObject = NULL;
        msg << "ERROR: can't find a friendly building" 
            << " to use as reachability source. Did we lose?\n";
      }
    }
  }

#ifdef USE_CANVAS
  Sorts::canvas.unregisterSGO(sObject);
#endif

  objectMap.erase(gameObj);
  msg << "deceased sgo: " << sObject << " id: " << id << endl;
  //delete sObject;
  sObject->removeFromGame();
  assert(liveIDs.find(id) != liveIDs.end());
  liveIDs.erase(id);
}
  
void OrtsInterface::removeVanishedObject(const GameObj* gameObj) {
  // just remove like a dead object for now, but change it later
  removeDeadObject(gameObj);
}

void OrtsInterface::removeDeadObjects() {
  // this must happen before assignActions, and updateSoarGameObjects must
  // happen after- this way, we won't have problems if we assign an action to
  // something that just died
  
  FORALL(changes.vanished_objs, obj) {
    GameObj* gob = (*obj)->get_GameObj();
    if (gob == 0) continue;
    if (gob->sod.in_game) {
      assert(objectMap.find(gob) != objectMap.end());
      requiredUpdatesNextCycle.erase(objectMap[gob]);
      removeVanishedObject(gob);

    }
  }
  
  FORALL(changes.dead_objs, obj) {
    GameObj* gob = (*obj)->get_GameObj();
    if (gob == 0) continue;
    if (gob->sod.in_game) {
      assert(objectMap.find(gob) != objectMap.end());
      requiredUpdatesNextCycle.erase(objectMap[gob]);
      removeDeadObject(gob);
    }
  }
}

void OrtsInterface::updateSoarGameObjects() {
  // an update can cause an object to insert itself back in the required
  // queue, so copy it out and clear it so we don't update each object 
  // more than once

  lastError = playerGameObj->get_int("errNum");
  if (lastError > 0) {
    msg << "ORTS reported error " << lastError << endl;
    playerGameObj->set_int("errNum", 0);
  }
  buildingThisCycle = false;
  
  set <SoarGameObject*> requiredThisCycle = requiredUpdatesNextCycle;
  requiredUpdatesNextCycle.clear();

  updateSoarPlayerInfo();
  
  
  FORALL(changes.new_boundaries, obj) {
    GameObj* gob = (*obj)->get_GameObj();
    Line l (*gob->sod.x1, *gob->sod.y1, *gob->sod.x2, *gob->sod.y2);
//    Sorts::terrainManager.addSegment
//      (*gob->sod.x1, *gob->sod.y1, *gob->sod.x2, *gob->sod.y2);
    Sorts::spatialDB->addTerrainLine(l);
#ifdef USE_CANVAS
    Sorts::canvas.drawLine(l.a.x, l.a.y, l.b.x, l.b.y);
    Sorts::canvas.update();
#endif
  }

  // add new objects
  FORALL(changes.new_objs, obj) {
    GameObj* gob = (*obj)->get_GameObj();
    if (gob == NULL) continue;
    if (gob->sod.in_game) {
      /* It's not clear whether this object was created or appeared, maybe we
       * should drop the distinction altogether, or do some extra bookkeeping
       * here
       */
      //addAppearedObject(gob);
      addCreatedObject(gob);
    }
  }

  FORALL(changes.changed_objs, obj) {
    GameObj* gob = (*obj)->get_GameObj();
    SoarGameObject* sgo;
    if (gob == NULL) {
      continue;
    }
    else if (gob->sod.in_game) {
      /* we should have a SoarGameObject for this GameObj, if not, we're in
       * trouble
       */
      assert(objectMap.find(gob) != objectMap.end());

      // just call update to let it know there were changes
      sgo = objectMap[gob];
      sgo->update();
      // if it is marked update-required for this cycle,
      // make sure it isn't double-updated
      requiredThisCycle.erase(sgo);
    }
    else if (gob == playerGameObj) {
      // we check this every cycle
    }
  }

  // issue required updates that didn't happen due to a change
  for (set<SoarGameObject*>::iterator it = requiredThisCycle.begin();
      it != requiredThisCycle.end();
      it++) {
    (*it)->update();
  }

  if (unitCountsStale) {
    unitCountsStale = false;
    Sorts::SoarIO->updatePlayerUnits(numWorkers, numTanks, numMarines);
  }
  
}

void OrtsInterface::updateMap() {
//  Sorts::mapManager->addExploredTiles(changes.new_tile_indexes);
//  Sorts::mapManager->addBoundaries(changes.new_boundaries);
}

void OrtsInterface::updateSoarPlayerInfo() {
  
  // only know get gold for now
  if (gold != playerGameObj->get_int("minerals")) {
    gold = playerGameObj->get_int("minerals");
    Sorts::SoarIO->updatePlayerGold(gold);
  }
}

OrtsInterface::~OrtsInterface() {
  objectMapIter it = objectMap.begin();
  while (it != objectMap.end()) {
    delete (*it).second;
    it++;
  }
}

void OrtsInterface::updateNextCycle(SoarGameObject* sgo) {
  requiredUpdatesNextCycle.insert(sgo);
}

bool OrtsInterface::isAlive(int id) {
  return (liveIDs.find(id) != liveIDs.end());
}

int OrtsInterface::getWorldId() {
  // could also be called getNumPlayers..
  return gsm->get_game().get_player_num();
}

int OrtsInterface::getNumPlayers() {
  // for ease of code reading
  return getWorldId();
}
int OrtsInterface::getMyId() {
  return myPid;
}

SoarGameObject* OrtsInterface::getSoarGameObject(GameObj* gob) {
  map<const GameObj*, SoarGameObject*>::iterator pos = objectMap.find(gob);
  if (pos == objectMap.end()) {
    return NULL;
  }
  else {
    return pos->second;
  }
}

double OrtsInterface::getOrtsDistance(GameObj* go1, GameObj* go2) {
  // return the distance between two objects as determined by ORTS
  return (double) gsm->get_game().distance(*go1, *go2);
}

int OrtsInterface::getViewFrame() {
  return viewFrame;
}

int OrtsInterface::getActionFrame() {
  return lastActionFrame;
}

int OrtsInterface::getMapXDim() {
  return mapXDim;
}

int OrtsInterface::getMapYDim() {
  return mapYDim;
}


void OrtsInterface::mergeChanges(GameChanges& newChanges) {
  // add the new gameChanges to the existing set, if any
  // this is needed since we can skip ORTS events, and the gameChanges
  // sent by the server are cleared every cycle.

  // must handle cases where objects appear and disappear without an unskipped
  // event consistently

  // new_objs: add all to changes
  // changed_objs: add all to changes, if not present
  // vanished_objs: if present in new/changed, rm from those, 
  //   otherwise add to changes
  // dead_objs: same
  // new_tile_indexes: add all to changes
  // new_boundaries: add all to changes
  Vector<ScriptObj*>::iterator it;
  Vector<ScriptObj*>::iterator it2;
  Vector<int>::iterator intIt;
  bool found;
  for (it = newChanges.new_objs.begin(); 
       it != newChanges.new_objs.end(); 
       it++) {
    changes.new_objs.push_back(*it);
  }
  for (intIt = newChanges.new_tile_indexes.begin(); 
       intIt != newChanges.new_tile_indexes.end(); 
       intIt++) {
    changes.new_tile_indexes.push_back(*intIt);
  }
  for (it = newChanges.new_boundaries.begin(); 
       it != newChanges.new_boundaries.end(); 
       it++) {
    changes.new_boundaries.push_back(*it);
  }

  for (it = newChanges.changed_objs.begin(); 
       it != newChanges.changed_objs.end(); 
       it++) {
    if (changes.changed_objs.find(*it) == changes.changed_objs.end()) {
      changes.changed_objs.push_back(*it);
    }
  }

  for (it = newChanges.vanished_objs.begin(); 
       it != newChanges.vanished_objs.end(); 
       it++) {
    found = false;
    it2 = changes.new_objs.find(*it);
    if (it2 != changes.new_objs.end()) {
      changes.new_objs.erase(it2);
      found = true;
    }
    it2 = changes.changed_objs.find(*it);
    if (it2 != changes.changed_objs.end()) {
      changes.changed_objs.erase(it2);
    }
    if (not found) {
      changes.vanished_objs.push_back(*it);
    }
  }
  for (it = newChanges.dead_objs.begin(); 
       it != newChanges.dead_objs.end(); 
       it++) {
    found = false;
    it2 = changes.new_objs.find(*it);
    if (it2 != changes.new_objs.end()) {
      changes.new_objs.erase(it2);
      found = true;
    }
    it2 = changes.changed_objs.find(*it);
    if (it2 != changes.changed_objs.end()) {
      changes.changed_objs.erase(it2);
    }
    if (not found) {
      changes.dead_objs.push_back(*it);
    }
  }
}


