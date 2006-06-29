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

  // SGO's use this to insert themselves in the list of required updates
  void updateNextCycle(SoarGameObject* sgo);
  
  // return true if the object with that ID is visible and alive
  bool isAlive(int id);

  int getWorldId();
  int getMyId();
  int getGobId(GameObj* gob) { return gsm->get_game().get_cplayer_info().get_id(gob); }
  SoarGameObject* getSoarGameObject(GameObj* gob);
  double getOrtsDistance(GameObj* go1, GameObj* go2);
  int getViewFrame();
  int getActionFrame();
  int getNumPlayers();

  int getMapXDim();
  int getMapYDim();

  int getSkippedActions() { return skippedActions; } 

  int getLastError() { return lastError; }
  void setBuildAction() { buildingThisCycle = true; }
  bool getBuildAction() { return buildingThisCycle; }

  int getCurrentMinerals() { return gold; }

  GameObj* getReachabilityObject() { return reachabilityObject; }
  int getDeadCount() { return deadCount; }
private:
  GameStateModule* gsm;

  // pointers to all the orts stuff
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
  void updateSoarGameObjects();
  void removeDeadObjects();
  void updateMap();
  void updateSoarPlayerInfo();
  void mergeChanges(GameChanges& newChanges);
  
  int counter;
  int gold;
  
  // player id of Soar
  int myPid;

  int skippedActions;
  int mapXDim, mapYDim;

  int lastActionFrame;
  int viewFrame;

  int lastError;
  bool buildingThisCycle;
  bool unitCountsStale;
  
  int numWorkers, numMarines, numTanks;
  // list of SGO's that need to be updated next cycle,
  // regardless of if they change in the world or not
  set <SoarGameObject*> requiredUpdatesNextCycle;
  set <int> liveIDs;
  
  GameChanges changes;

  list <const GameObj*> friendlyBuildings;
  
  GameObj* reachabilityObject;
  int deadCount;
};

#endif
