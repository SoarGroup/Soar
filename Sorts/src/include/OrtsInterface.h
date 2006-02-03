#ifndef orts_interface
#define orts_interface

#include<hash_map.h>

// our includes
#include "SoarInterface.h"
#include "SoarGameObj.h"
#include "GroupManager.h"

// sml includes
#include "sml_Client.h"

// orts includes
#include "Global.H"
#include "Game.H"
#include "GameStateModule.H"
#include "GameChanges.H"
#include "GameObj.H"

using namespace std;

typedef hash_map<GameObj*, SoarGameObject*>::iterator objectMapIter;
typedef hash_map<SoarGameObject*, GameObj*>::iterator revObjectMapIter;

/* The class that does most of the interfacing with ORTS, as well as
 * handle events from it
 */
class OrtsInterface : public EventHandler {
public:
  OrtsInterface(GameStateModule* _gsm, SoarInterface *_soarInterface);
  ~OrtsInterface();

private:
  GroupManager& groupManager;
  
  // pointers to all the orts stuff
  GameStateModule* gsm;
  GameObj *playerGameObj;

  // keep track of all game objects
  hash_map<GameObj*, SoarGameObject*> objectMap;
  hash_map<SoarGameObject*, GameObj*> revObjectMap;

  // pointer to our stuff
  SoarInterface* soarInterface;

  // consistency functions
  void addAppearedObject(const GameObj* gameObj);
  void addCreatedObject(const GameObj* gameObj);
  void removeDeadObject(const GameObj* gameObj);
  void removeVanishedObject(const GameObj* gameObj);

  // event handler functions
  void handle_event(const Event& e);
  void updateSoarGameObjects(const GameChanges& changes);
  void updateSoarPlayerInfo();
};

#endif
