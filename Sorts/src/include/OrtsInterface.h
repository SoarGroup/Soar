#ifndef orts_interface
#define orts_interface

// our includes
#include "SoarGameObject.h"
class Sorts;

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
  OrtsInterface(GameStateModule* _gsm);

  ~OrtsInterface();

  void setSorts(const Sorts* _sorts) { sorts = _sorts; }

  // SGO's use this to insert themselves in the list of required updates
  void updateNextCycle(SoarGameObject* sgo);
  
  // return true if the object with that ID is visible and alive
  bool isAlive(int id);

  int getWorldId();
  int getMyId();
  double getOrtsDistance(GameObj* go1, GameObj* go2);
  int getFrameID();
  int getNumPlayers();

  int getMapXDim();
  int getMapYDim();

private:
  const Sorts* sorts;

  // pointers to all the orts stuff
  GameStateModule* gsm;
  GameObj *playerGameObj;

  // keep track of all game objects
  /* In the future, change these to hash maps */
  map<const GameObj*, SoarGameObject*> objectMap;
  map<SoarGameObject*, const GameObj*> revObjectMap;

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

  int mapXDim, mapYDim;

  // list of SGO's that need to be updated next cycle,
  // regardless of if they change in the world or not
  set <SoarGameObject*> requiredUpdatesNextCycle;
  set <int> liveIDs;
};

#endif
