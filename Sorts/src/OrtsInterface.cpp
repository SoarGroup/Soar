#include <assert.h>

#include "OrtsInterface.h"
#include "SoarGameGroup.h"

#include "GameObj.H"
#include "GameStateModule.H"

OrtsInterface::OrtsInterface( GameStateModule* _gsm,
                              SoarInterface*   _soarInterface,
                              GroupManager*    _groupManager,
                              MapManager*      _mapManager )
: gsm(_gsm), 
  soarInterface(_soarInterface), 
  groupManager(_groupManager),
  mapManager(_mapManager)
{
  counter = 0;
  myPid = -1; // this must be set after we connect to the server
}

void OrtsInterface::addAppearedObject(const GameObj* gameObj) {
  assert(false);
}

void OrtsInterface::addCreatedObject(GameObj* gameObj) {
  // make sure the game object does not exist in the middleware
  assert(objectMap.find(gameObj) == objectMap.end());
 
  bool friendly = (myPid == *gameObj->sod.owner);
  bool world    = (gsm->get_game().get_player_num() == *gameObj->sod.owner);
  SoarGameObject* newObj = new SoarGameObject(gameObj, friendly, world);
 
  // GroupManager takes care of setting the object->group pointers
  groupManager->addGroup(newObj);
  
  // more initializations of the object?

  objectMap[gameObj] = newObj;
}

void OrtsInterface::removeDeadObject(const GameObj* gameObj) {
  // make sure the game object exists
  assert(objectMap.find(gameObj) != objectMap.end());

  SoarGameObject* sObject = objectMap[gameObj];
  sObject->getGroup()->removeUnit(sObject);
  
  delete objectMap[gameObj];
  objectMap.erase(gameObj);
}
  
void OrtsInterface::removeVanishedObject(const GameObj* gameObj) {
  // just remove like a dead object for now, but change it later
  removeDeadObject(gameObj);
}

bool OrtsInterface::handle_event(const Event& e) {
  if (e.get_who() == GameStateModule::FROM) {
    if (e.get_what() == GameStateModule::VIEW_MSG) {

      groupManager->assignActions();

      const GameChanges& changes = gsm->get_changes();
      updateMap(changes);
      updateSoarGameObjects(changes);

      // since the FSM's have been updated, we should send the actions here
      gsm->send_actions();
      cout << "send_actions" << endl;

      // what was this for again?
      groupManager->updateWorld();

      /* I'm assuming here that those update calls from above have already
       * updated the soar input link correctly, so commit everything
       */
      soarInterface->commitInputLinkChanges();
    }
    return true;
  }
  else {
    return false;
  }
}

void OrtsInterface::updateSoarGameObjects(const GameChanges& changed) {
  set <SoarGameObject*> requiredNextCycle;
  
  // add new objects
  FORALL(changed.new_objs, obj) {
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

  FORALL(changed.changed_objs, obj) {
    GameObj* gob = (*obj)->get_GameObj();
    SoarGameObject* sgo;
    if (gob == 0) {
      continue;
    }
    else if (gob->sod.in_game) {
      /* we should have a SoarGameObject for this GameObj, if not, we're in
       * trouble
       */
       cout << "updating an object: " << (int) gob << endl;
      assert(objectMap.find(gob) != objectMap.end());

      // just call update to let it know there were changes
      sgo = objectMap[gob];
      sgo->update();
      // if it is marked update-required for this cycle,
      // make sure it isn't double-updated
      requiredUpdates.erase(sgo);
      if (sgo->getUpdateRequired()) {
        // this object must be updated next cycle
        requiredNextCycle.insert(sgo);
      }
    }
    else if (gob == playerGameObj) {
      updateSoarPlayerInfo();
    }
  }

  FORALL(changed.vanished_objs, obj) {
    GameObj* gob = (*obj)->get_GameObj();
    if (gob == 0) continue;
    if (gob->sod.in_game) {
      /* we should have a SoarGameObject for this GameObj, if not, we're in
       * trouble
       */
      assert(objectMap.find(gob) != objectMap.end());
      requiredUpdates.erase(objectMap[gob]);
      removeVanishedObject(gob);

      assert(requiredNextCycle.erase(objectMap[gob]) == 0);
      // if this assertion fails, just add requiredNextCycle.erase(sgo);
      // it just means my assumptions were wrong, and the same object
      // can appear as changed and vanished
    }
  }
  
  FORALL(changed.dead_objs, obj) {
    GameObj* gob = (*obj)->get_GameObj();
    if (gob == 0) continue;
    if (gob->sod.in_game) {
      /* we should have a SoarGameObject for this GameObj, if not, we're in
       * trouble
       */
      assert(objectMap.find(gob) != objectMap.end());
      requiredUpdates.erase(objectMap[gob]);
      removeDeadObject(gob);
      assert(requiredNextCycle.erase(objectMap[gob]) == 0);
      // see assertion note above
    }
  }

  // issue required updates
  // removeVanished/Dead objects should have removed anything that
  // just disappeared from this list
  for (set<SoarGameObject*>::iterator it = requiredUpdates.begin();
      it != requiredUpdates.end();
      it++) {
    (*it)->update();
    if ((*it)->getUpdateRequired()) {
      requiredNextCycle.insert(*it);
    }
  }

  requiredUpdates = requiredNextCycle;
}

void OrtsInterface::updateMap(const GameChanges& changed) {
  mapManager->addExploredTiles(changed.new_tile_indexes);
  mapManager->addBoundaries(changed.new_boundaries);
}

void OrtsInterface::updateSoarPlayerInfo() {
  // only know get gold for now
  soarInterface->updatePlayerGold(playerGameObj->get_int("gold"));
}

OrtsInterface::~OrtsInterface() {
  objectMapIter it = objectMap.begin();
  while (it != objectMap.end()) {
    delete (*it).second;
    it++;
  }
}

sint4 OrtsInterface::getID(SoarGameObject* obj) {
  // this is needed at least for the mineral-gather command
  // translate the SGO into the parameter for set_action

  // as seen in libs/ai/low/src/GatherAI.C:
  // gsm.get_game().get_cplayer_info().get_id(gd.res)
  // where gd.res is a GameObj*
  // this ID is then put as the only parameter to a "gather" command
  
  return gsm->get_game().get_cplayer_info().get_id(obj->gob);
}
void OrtsInterface::setMyPid(int pid) {
  myPid = pid;
}

void OrtsInterface::updateNextCycle(SoarGameObject* sgo) {
  requiredUpdates.insert(sgo);
}
