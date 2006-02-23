#ifndef orts_interface
#define orts_interface

#include<hash_map.h>

// our includes
#include "SoarInterface.h"
#include "SoarGameObject.h"
#include "GroupManager.h"
#include "MapManager.h"

// sml includes
#include "sml_Client.h"

// orts includes
#include "Global.H"
#include "Game.H"
#include "GameStateModule.H"
#include "GameChanges.H"
#include "GameObj.H"

using namespace std;

typedef map<const GameObj*, SoarGameObject*>::iterator objectMapIter;
typedef map<SoarGameObject*, const GameObj*>::iterator revObjectMapIter;

/* The class that does most of the interfacing with ORTS, as well as
 * handle events from it
 */
class OrtsInterface : public EventHandler {
public:
  OrtsInterface( GameStateModule* _gsm, 
                 SoarInterface*   _soarInterface, 
                 GroupManager*    _groupManager,
                 MapManager*      _mapManager );

  ~OrtsInterface();

  void setMyPid(int pid);

  // SGO's use this to insert themselves in the list of required updates
  void updateNextCycle(SoarGameObject* sgo);
  
  // return true if the object with that ID is visible and alive
  bool isAlive(int id);

private:
  // pointers to all the orts stuff
  GameStateModule* gsm;
  GameObj *playerGameObj;

  // keep track of all game objects
  /* In the future, change these to hash maps */
  map<const GameObj*, SoarGameObject*> objectMap;
  map<SoarGameObject*, const GameObj*> revObjectMap;

  // pointer to our stuff
  SoarInterface* soarInterface;
  GroupManager*  groupManager;
  MapManager*    mapManager;

  // consistency functions
  void addAppearedObject(const GameObj* gameObj);
  void addCreatedObject(GameObj* gameObj);
  void removeDeadObject(const GameObj* gameObj);
  void removeVanishedObject(const GameObj* gameObj);

  // event handler functions
  bool handle_event(const Event& e);
  void updateSoarGameObjects(const GameChanges& changes);
  void updateMap(const GameChanges& changes);
  void updateSoarPlayerInfo();
  
  int counter;
  
  // player id of Soar
  int myPid;

  // list of SGO's that need to be updated next cycle,
  // regardless of if they change in the world or not
  set <SoarGameObject*> requiredUpdatesNextCycle;
  set <int> liveIDs;
};

#endif
