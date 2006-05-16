#include <assert.h>

// our includes
#include "Sorts.h"
#include "PerceptualGroup.h"
#include "InternalGroup.h"
#include "MineManager.h"

// Orts includes
#include "GameObj.H"
#include "GameStateModule.H"

OrtsInterface::OrtsInterface(GameStateModule* _gsm)
: gsm(_gsm)
{
  counter = 0;
  // gsm must have already connected for this to work
  myPid = gsm->get_game().get_client_player(); 
  mapXDim = (gsm->get_game().get_gtiles_x())*(gsm->get_game().get_tile_points());
  mapYDim = (gsm->get_game().get_gtiles_y())*(gsm->get_game().get_tile_points());
  lastActionFrame = 0;
  viewFrame = -1;
}

bool OrtsInterface::handle_event(const Event& e) {
  pthread_mutex_lock(Sorts::mutex);
  if (e.get_who() == GameStateModule::FROM) {
    if (e.get_what() == GameStateModule::VIEW_MSG) {
      cout << "ORTS EVENT {\n";
      viewFrame = gsm->get_game().get_view_frame();
      Sorts::SoarIO->updateViewFrame(viewFrame);
      int aFrame = gsm->get_game().get_action_frame();
        
      mergeChanges(const_cast<GameChanges&> (gsm->get_changes())); 
      // cleared every event, so we need to merge the changes for skipped
      // events

      // if you can figure out how to iterate through a vector inside a
      // const'd class, feel free to fix the const_cast
      
      if (aFrame != -1) {
        lastActionFrame = aFrame;
      }
      cout << "event for frame " 
           << viewFrame << "/" << lastActionFrame << endl;
      int skipped = gsm->get_game().get_skipped_actions();
      if (skipped) {
        cout << "WARNING: skipped actions: " << skipped << endl;
      }
      if ((viewFrame - lastActionFrame) > 10) {
        Sorts::catchup = true;
        cout << "client is behind, skipping event. v: "
             << viewFrame << " a:" << lastActionFrame << "\n";
        gsm->send_actions(); // empty, needed by server
        pthread_mutex_unlock(Sorts::mutex);
        return true;
      }
      
      Sorts::catchup = false;
      
      int merged = gsm->get_game().get_merged_actions();
      if (merged) {
        // these appear to be harmless
        //  cout << "WARNING: merged actions (what does this mean?)" << merged << endl;
      }

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
    }
    cout << "ORTS EVENT }\n";
    pthread_mutex_unlock(Sorts::mutex);
    return true;
  }
  else {
    pthread_mutex_unlock(Sorts::mutex);
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
  
  SoarGameObject* newObj = new SoarGameObject(gameObj,
                                              friendly, world, id);
 
  // PerceptualGroupManager takes care of setting the object->group pointers
  Sorts::pGroupManager->makeNewGroup(newObj);
  Sorts::iGroupManager->makeNewGroup(newObj);

  if (gameObj->bp_name() == "mineral") {
    Sorts::mineManager->addMineral(newObj);
  }
  else if (friendly && gameObj->bp_name() == "controlCenter") {
    Sorts::mineManager->addControlCenter(newObj);
  }
  

  objectMap[gameObj] = newObj;
   
  /*if (liveIDs.find(id) != liveIDs.end()) {
    cout << "ERROR: appeard object is there already: " << (int)gameObj << endl;
  }*/
  assert(liveIDs.find(id) == liveIDs.end());
  liveIDs.insert(id);
}

void OrtsInterface::removeDeadObject(const GameObj* gameObj) {
  // make sure the game object exists
  assert(objectMap.find(gameObj) != objectMap.end());

  SoarGameObject* sObject = objectMap[gameObj];
  int id = sObject->getID();
  sObject->getPerceptualGroup()->removeUnit(sObject);
  if(sObject->getInternalGroup()!=NULL)
   sObject->getInternalGroup()->removeUnit(sObject);
  
  if (gameObj->bp_name() == "mineral") {
    Sorts::mineManager->removeMineral(sObject);
  }
  else if (gameObj->bp_name() == "controlCenter") {
    Sorts::mineManager->removeControlCenter(sObject);
  }
  
  delete objectMap[gameObj];
  objectMap.erase(gameObj);
  liveIDs.erase(id);
}
  
void OrtsInterface::removeVanishedObject(const GameObj* gameObj) {
  // just remove like a dead object for now, but change it later
  removeDeadObject(gameObj);
}


void OrtsInterface::updateSoarGameObjects() {
  // an update can cause an object to insert itself back in the required
  // queue, so copy it out and clear it so we don't update each object 
  // more than once
  
  set <SoarGameObject*> requiredThisCycle = requiredUpdatesNextCycle;
  requiredUpdatesNextCycle.clear();
  
  // add new objects
  FORALL(changes.new_objs, obj) {
    GameObj* gob = (*obj)->get_GameObj();
    if (gob == 0) continue;
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
    if (gob == 0) {
      continue;
    }
    else if (gob->sod.in_game) {
      /* we should have a SoarGameObject for this GameObj, if not, we're in
       * trouble
       */
      //cout << "updating an object: " << (int) gob << endl;
      assert(objectMap.find(gob) != objectMap.end());

      // just call update to let it know there were changes
      sgo = objectMap[gob];
      sgo->update();
      // if it is marked update-required for this cycle,
      // make sure it isn't double-updated
      requiredThisCycle.erase(sgo);
    }
    else if (gob == playerGameObj) {
      updateSoarPlayerInfo();
    }
  }

  FORALL(changes.vanished_objs, obj) {
    GameObj* gob = (*obj)->get_GameObj();
    if (gob == 0) continue;
    if (gob->sod.in_game) {
      /* we should have a SoarGameObject for this GameObj, if not, we're in
       * trouble
       */
      assert(objectMap.find(gob) != objectMap.end());
      requiredUpdatesNextCycle.erase(objectMap[gob]);
      requiredThisCycle.erase(objectMap[gob]);
      removeVanishedObject(gob);

    }
  }
  
  FORALL(changes.dead_objs, obj) {
    GameObj* gob = (*obj)->get_GameObj();
    if (gob == 0) continue;
    if (gob->sod.in_game) {
      /* we should have a SoarGameObject for this GameObj, if not, we're in
       * trouble
       */
      assert(objectMap.find(gob) != objectMap.end());
      requiredUpdatesNextCycle.erase(objectMap[gob]);
      requiredThisCycle.erase(objectMap[gob]);
      removeDeadObject(gob);
    }
  }

  // issue required updates that didn't happen due to a change
  for (set<SoarGameObject*>::iterator it = requiredThisCycle.begin();
      it != requiredThisCycle.end();
      it++) {
    (*it)->update();
  }
  
}

void OrtsInterface::updateMap() {
  Sorts::mapManager->addExploredTiles(changes.new_tile_indexes);
  Sorts::mapManager->addBoundaries(changes.new_boundaries);
}

void OrtsInterface::updateSoarPlayerInfo() {
  // only know get gold for now
  Sorts::SoarIO->updatePlayerGold(playerGameObj->get_int("gold"));
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

double OrtsInterface::getOrtsDistance(GameObj* go1, GameObj* go2) {
  // return the distance between two objects as determined by ORTS
  // use sparingly (does this go over the network?)
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
  bool found;
  for (it = newChanges.new_objs.begin(); 
       it != newChanges.new_objs.end(); 
       it++) {
    changes.new_objs.push_back(*it);
  }

  for (it = newChanges.changed_objs.begin(); 
       it != newChanges.changed_objs.end(); 
       it++) {
    if (changes.changed_objs.find(*it) != changes.changed_objs.end()) {
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
      found = true;
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
      found = true;
    }
    if (not found) {
      changes.dead_objs.push_back(*it);
    }
  }
}
