#include <assert.h>
#include "OrtsInterface.h"

OrtsInterface::addAppearedObject() {

}

OrtsInterface::addCreatedObject(const GameObj* gameObj) {
  // make sure the game object does not exist in the middleware
  assert(objectMap.find(gameObj) == objectMap.end());

  SoarGameObject* newObj = new SoarGameObject();
  newObj->group = groupManager.addGroup(newObj);
  // more initializations of the object?

  objectMap[gameObj] = newObj;
}

OrtsInterface::removeDeadObject(const GameObj* gameObj) {
  // make sure the game object exists
  assert(objectmap.find(gameObj) != objectMap.end());

  groupManager.removeFromGroup(objectMap[gameObj]);
  delete objectMap[gameObj];
  objectMap.erase(gameObj);
}

bool OrtsInterface::handle_event(const Event& e) {
  if (e.get_who() == GameStateModule::FROM) {
    if (e.get_what() == GameStateModule::VIEW_MSG) {
      const GameChanges& changes = gsm->get_changes();

      updateSoarGameObjects(changes);

      /* I'm assuming here that those update calls from above have already
       * updated the soar input link correctly, so commit everything
       */
      agent->Commit();
    }
  }
}

void OrtsInterface::updateSoarGameObjects(const GameChanges& changes) {
  // add new objects
  FORALL(changed.new_objs, obj) {
    GameObj* gob = (*obj)->get_GameObj();
    if (gob == 0) continue;
    if (gob->sod.in_game) {
      /* It's not clear whether this object was created or appeared, maybe we
       * should drop the distinction altogether, or do some extra bookkeeping
       * here
       */
      addAppearedObject(gob);
    }
  }

  FORALL(changed.changed_objs, obj) {
    GameObj* gob = (*obj)->get_GameObj();
    if (gob == 0) {
      continue;
    }
    else if (gob->sod.in_game) {
      /* we should have a SoarGameObject for this GameObj, if not, we're in
       * trouble
       */
      assert(objectMap.find(gob) != objectMap.end());

      // just call update to let it know there were changes
      objectMap[gob]->update();
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

      removeVanishedObject(gob);
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

      removeDeadObject(gob);
    }
  }
}

void OrtsInterface::updateSoarPlayerInfo() {
  // only know get gold for now
  soarInterface->updatePlayerGold(playerGameObj->get_int("gold"));
}
