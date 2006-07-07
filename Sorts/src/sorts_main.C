/*
    This file is part of Sorts, an interface between Soar and ORTS.
    (c) 2006 James Irizarry, Sam Wintermute, and Joseph Xu

    Sorts is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Sorts is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Sorts; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA    
*/
#include<iostream>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>

#include <SDL/SDL.h>

//#include "sml_Connection.h"
#include "sml_Client.h"

// our includes
#include "SoarInterface.h"
#include "OrtsInterface.h"
#include "MapManager.h"
#include "AttackManagerRegistry.h"
#include "MineManager.h"
#include "GridMapTileGrouper.h"
#include "FeatureMapManager.h"
#include "GameActionManager.h"
#include "Sorts.h"

#include "TerrainModule.H"
#include "Demo_SimpleTerrain.H"

#define msg cout << "MAIN: "

#define MAX_SOAR_AHEAD_CYCLES 5

#define SOAR_862

using namespace sml;

bool useSoarStops;

void printOutput
( smlPrintEventId id,
  void*           pUserData, 
  Agent*          pAgent,
  const char*     pMessage)
{
  std::cout << "SOARINT: " << pMessage << std::endl;
#ifdef USE_CANVAS
 /* pthread_mutex_lock(Sorts::mutex);
  string s(pMessage);
  Sorts::canvas.setSoarStatus(s);
  pthread_mutex_unlock(Sorts::mutex);*/
#endif
}

/* We have two different threads giving us events, Soar and ORTS.

  On Soar events (below):
    get output from agent, add commands to queues to be processed
    process vision-only commands immediately
    refresh Soar's input with changes in vision / the ORTS world

  On ORTS events (OrtsInterface::handle_event):
    apply Soar's object action commands to the ORTS objects
    gather changes from the ORTS server
    update the map
    update each game object
    send game object actions to ORTS
    refresh Soar's view of the world (but don't commit)
*/


#ifdef SOAR_862

void SoarStartRunEventHandler
( smlRunEventId id, 
  void*         pUserData, 
  Agent*        agent, 
  smlPhase      phase )
{
  pthread_mutex_unlock(Sorts::mutex);
  //Sorts::SoarIO->unlockSoarMutex();  
}

void SoarAfterDecisionCycleEventHandler
( smlRunEventId id, 
  void*         pUserData, 
  Agent*        agent, 
  smlPhase      phase )
{
  if (useSoarStops) {
    if (!Sorts::SoarIO->isSoarRunning()) {
      agent->StopSelf();
    }
  }
}

void SoarOutputEventHandler
( smlRunEventId id, 
  void*         pUserData, 
  Agent*        agent, 
  smlPhase      phase )
{
  unsigned long st = gettime();
  if (Sorts::cyclesSoarAhead > 5) {
    Sorts::SoarIO->stopSoar();
  }
  Sorts::cyclesSoarAhead++;

  pthread_mutex_lock(Sorts::mutex);
  cout << "SOAR EVENT {\n";
  
  if (Sorts::catchup == true) {
    cout << "ignoring Soar event, ORTS is behind.\n";
    pthread_mutex_unlock(Sorts::mutex);
   // msg << "TIME " << (gettime() - st) / 1000 << endl;
    return;
  }
  Sorts::SoarIO->getNewSoarOutput();

  // vision commands must be processed here- these are swapping
  // in and out the groups on the input link.
  // if they were processed in the ORTS handler, the agent
  // would have a delay from when it looked somewhere and when
  // the objects there appeared.
  Sorts::pGroupManager->processVisionCommands();
  Sorts::gameActionManager->processGameCommands();

  if (Sorts::SoarIO->getStale()) {
    // why do we have these here?
    //Sorts::SoarIO->lockSoarMutex();
    agent->Commit();
    //Sorts::SoarIO->unlockSoarMutex();
    Sorts::SoarIO->setStale(false);
  }
  cout << "SOAR EVENT }" << endl;
 // msg << "TIME " << (gettime() - st) / 1000 << endl;
  pthread_mutex_unlock(Sorts::mutex);
}
#else
void SoarUpdateEventHandler(smlUpdateEventId id, 
                            void*                 pUserData,
                            Kernel*          pKernel,
                            smlRunFlags      runFlags) {
  pthread_mutex_lock(Sorts::mutex);
  std::cout << "SOAR EVENT {\n";
  if (Sorts::catchup == true) {
    std::cout << "ignoring Soar event, ORTS is behind.\n";
    pthread_mutex_unlock(Sorts::mutex);
    msg << "TIME " << (gettime() - st) / 1000 << endl;
    return;
  }
  Agent *agent = pKernel->GetAgent("orts_agent");
  
  Sorts::SoarIO->getNewSoarOutput();

  // vision commands must be processed here- these are swapping
  // in and out the groups on the input link.
  // if they were processed in the ORTS handler, the agent
  // would have a delay from when it looked somewhere and when
  // the objects there appeared.
  Sorts::pGroupManager->processVisionCommands();
  Sorts::mapQuery->processMapCommands();

  if (Sorts::SoarIO->getStale()) {
    Sorts::SoarIO->lockSoarMutex();
    agent->Commit();
    Sorts::SoarIO->unlockSoarMutex();
    Sorts::SoarIO->setStale(false);
  }
  std::cout << "SOAR EVENT }\n";
  pthread_mutex_unlock(Sorts::mutex);
}
#endif

// the function that is executed by a separate thread to
// run the Soar agent
void* RunSoar(void* ptr) {
  /* For some reason if this delay isn't here, this thread will
   * grab the CPU and never let it go
   */
  sleep(1);

  if (useSoarStops) {
    while (true) {
#ifdef SOAR_862
      pthread_mutex_lock(Sorts::mutex);
    //  Sorts::SoarIO->lockSoarMutex();
      // to be unlocked upon the start event
      ((Agent*) ptr)->Commit();
      ((Agent*) ptr)->RunSelfForever();
#else
      ((Kernel*) ptr)->RunAllAgentsForever(sml_INTERLEAVE_DECISION);
#endif
      // spin until Soar gets started again
      while (!Sorts::SoarIO->isSoarRunning()) {
        usleep(60000); // assuming 8 fps from server
      }
    }
  }
  else {
#ifdef SOAR_862
    pthread_mutex_lock(Sorts::mutex);
    // to be unlocked upon the start event
    ((Agent*) ptr)->Commit();
    ((Agent*) ptr)->RunSelfForever();
#else
    ((Kernel*) ptr)->RunAllAgentsForever(sml_INTERLEAVE_DECISION);
#endif
  }
 
  // just to keep the compiler from warning
  return NULL;
}

void* RunOrts(void* ptr) {
  GameStateModule* gsm = (GameStateModule*) ptr;
  msg << "Starting ORTS" << endl;
  while(1) {
//  for(unsigned long i = 0; i < 1000; i++) {
    if (!gsm->recv_view()) {
      usleep(30000);
    }
  }
  exit(0);
  // unreachable (what is this supposed to return, anyway?)
  return NULL;
}

typedef Demo_SimpleTerrain::ST_Terrain Terrain;

int main(int argc, char *argv[]) {
#ifdef USE_CANVAS
  atexit(SDL_Quit);
#endif

  int port = 7777;
  int soarPort = 12121;
  int seed = 0;
  string host = "127.0.0.1";
  string id = "a";
  char* productions = NULL;
  useSoarStops = true;

  bool printSoar = false;

  for(int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-port") == 0) {
      port = atoi(argv[i+1]);
    }
    else if (strcmp(argv[i], "-host") == 0) {
      host = argv[i+1];
    }
    else if ((strcmp(argv[i], "-productions") == 0)
            || (strcmp(argv[i], "-p") == 0)) {
      productions = argv[i+1];
    }
    else if (strcmp(argv[i], "-soar-port") == 0) {
      soarPort = atoi(argv[i+1]);
    }
    else if (strcmp(argv[i], "-seed") == 0) {
      seed = atoi(argv[i+1]);
    }
    else if (strcmp(argv[i], "-no-stop") == 0) {
      useSoarStops = false;
    }
    else if (strcmp(argv[i], "-id") == 0) {
      id = argv[i+1];
    }
    else if (strcmp(argv[i], "-print-soar") == 0) {
      printSoar = true;
    }
  }

  srand(seed);
  

  /************************************
   * Create mutexes for action queues *
   ************************************/
  pthread_mutex_t objectActionMutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_t attentionActionMutex = PTHREAD_MUTEX_INITIALIZER;
  
  pthread_mutex_t soarMutex = PTHREAD_MUTEX_INITIALIZER;
 
  pthread_mutex_t sortsMutex = PTHREAD_MUTEX_INITIALIZER;
  std::cout << "about to init sml\n";

  /*******************
   * Init sml client *
   *******************/
  Kernel* pKernel = Kernel::CreateKernelInNewThread("SoarKernelSML", soarPort) ;
  std::cout << "done\n";

  // Check that nothing went wrong.  We will always get back a kernel object
  // even if something went wrong and we have to abort.
  if (pKernel->HadError()) {
    std::cout << pKernel->GetLastErrorDescription() << std::endl;
    return 1;
  }

  // Create a Soar agent named test
  // NOTE: We don't delete the agent pointer.  It's owned by the kernel
  Agent* pAgent = pKernel->CreateAgent("orts_agent") ;

  // Check that nothing went wrong
  // NOTE: No agent gets created if thereâs a problem, so we have to check foor
  // errors through the kernel object.
  if (pKernel->HadError()) {
    std::cout << pKernel->GetLastErrorDescription() << std::endl ;
    return 1;
  }


  // Load some productions
  if (productions != NULL) {
    pAgent->LoadProductions(productions) ;
  }
  else {
    cout << "ERROR: No productions specified. Use -p or -productions.\n";
    exit(1);
  }

  if (pAgent->HadError()) {
    cout << "Soar agent reported an error:\n";
    cout << pAgent->GetLastErrorDescription() << endl ;
    return 1;
  }

  /*********************************
   * Set up the connection to ORTS *
   *********************************/
  GameStateModule::Options::add();
  Terrain::add_options();

  GameStateModule::Options gsmo;
  gsmo.port = port;
  gsmo.host = host;
  gsmo.id = id;
  GameStateModule gsm(gsmo);

  Terrain timp;
  TerrainModule tm(gsm, timp);
  //gsm.add_handler(&tm);
  
  // connect to ORTS server
  if (!gsm.connect()) exit(10);
  std::cout << "connected" << std::endl;
  
  Game& game = gsm.get_game();
  const Map<GameTile>& m = game.get_map();
  int gridSizeX = m.get_width() / 2;
  int gridSizeY = m.get_height() / 2;


  std::cout <<"calling..\n";
  
  SoarInterface soarInterface
  ( pAgent,
    &objectActionMutex,
    &attentionActionMutex,
    &soarMutex);

  // map manager, using the grid tile grouping method
  GridMapTileGrouper 
    tileGrouper(game.get_map(), game.get_tile_points(), gridSizeX, gridSizeY);
  MapManager mapManager
    ( game.get_map(), game.get_tile_points(), tileGrouper);
 
  FeatureMapManager featureMapManager;
  PerceptualGroupManager pgm;
  //InternalGroupManager igm;
  OrtsInterface ortsInterface(&gsm);

  GameActionManager gaMan;
  SpatialDB spatialDB;
  MineManager mineMan;
  AttackManagerRegistry amr;

  Sorts sorts(&soarInterface, 
              &ortsInterface, 
              &pgm, 
              //&igm,
              &mapManager, 
              &featureMapManager,
              &tm,
              &spatialDB,
              &amr,
              &mineMan,
              &gaMan,
              &sortsMutex);

  spatialDB.init();
  
  // must be connected to orts server by now
  // must initialize the gm before soar input link
  // pgm intialize gets params from orts server via ortsinterface
  // and calculates the initial vision params (window size, etc.)
  // these params are then put on the input link when it is initialized
  pgm.initialize();
  soarInterface.initSoarInputLink();

#ifdef USE_CANVAS
  // initialize the canvas
  Sorts::canvas.init
    ( ortsInterface.getMapXDim(), 
      ortsInterface.getMapYDim(),
      1.2 );
     // 1.7 );
#endif

// register for all events
  gsm.add_handler(&ortsInterface);

#ifdef SOAR_862
  pAgent->RegisterForRunEvent(smlEVENT_BEFORE_RUN_STARTS, SoarStartRunEventHandler, NULL);
  pAgent->RegisterForRunEvent(smlEVENT_AFTER_OUTPUT_PHASE, SoarOutputEventHandler, NULL);
  pAgent->RegisterForRunEvent(smlEVENT_AFTER_DECISION_CYCLE, SoarAfterDecisionCycleEventHandler, NULL);
 // pAgent->RegisterForRunEvent(smlEVENT_BEFORE_INPUT_PHASE, SoarInputEventHandler, &sorts);
#else
  pKernel->RegisterForUpdateEvent(smlEVENT_AFTER_ALL_OUTPUT_PHASES, SoarUpdateEventHandler, &sorts);
#endif

  if (printSoar) {
    pAgent->RegisterForPrintEvent(smlEVENT_PRINT, printOutput, &sorts);  
  }
    
  // start Soar in a different thread
  pthread_attr_t soarThreadAttribs;
  pthread_attr_init(&soarThreadAttribs);
  pthread_t soarThread;
  
#ifdef SOAR_862
  pKernel->SetAutoCommit(false);
  pthread_create(&soarThread, &soarThreadAttribs, RunSoar, (void*) pAgent);
#else
  pthread_create(&soarThread, &soarThreadAttribs, RunSoar, (void*) pKernel);
#endif

  // this drives the orts interrupt, which drives the middleware
  pthread_attr_t ortsThreadAttribs;
  pthread_attr_init(&ortsThreadAttribs);
  pthread_t ortsThread;
  pthread_create(&ortsThread, &ortsThreadAttribs, RunOrts, (void*) &gsm);
  std::cout << "ORTS client is running" << std::endl;

  pthread_join(soarThread, NULL);
  pthread_join(ortsThread, NULL);

  return 0;
}
