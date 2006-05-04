#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

#include "sml_Client.h"

// our includes
#include "SoarInterface.h"
#include "OrtsInterface.h"
#include "MapManager.h"
#include "GridMapTileGrouper.h"
#include "FeatureMapManager.h"
#include "Sorts.h"

#include "TerrainModule.H"
#include "SimpleTerrain.H"


using namespace std;

void printOutput(sml::smlPrintEventId id,
                 void*                pUserData,
                 sml::Agent*          pAgent,
                 const char*          pMessage)
{
  cout << "[SOAR] " << pMessage << endl;
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


void SoarUpdateEventHandler(sml::smlUpdateEventId id, 
                            void*                 pUserData,
                            sml::Kernel*          pKernel,
                            sml::smlRunFlags      runFlags)
{
  //Sorts* sorts = (Sorts*) pUserData;
  pthread_mutex_lock(Sorts::mutex);
  cout << "SOAR EVENT {\n";
  if (Sorts::catchup == true) {
    cout << "ignoring Soar event, ORTS is behind.\n";
    pthread_mutex_unlock(Sorts::mutex);
    return;
  }
  sml::Agent *agent = pKernel->GetAgent("orts_agent");
  
  Sorts::SoarIO->getNewSoarOutput();

  // vision commands must be processed here- these are swapping
  // in and out the groups on the input link.
  // if they were processed in the ORTS handler, the agent
  // would have a delay from when it looked somewhere and when
  // the objects there appeared.
  Sorts::groupManager->processVisionCommands();

  if (Sorts::SoarIO->getStale()) {
    Sorts::SoarIO->lockSoarMutex();
    agent->Commit();
    Sorts::SoarIO->unlockSoarMutex();
    Sorts::SoarIO->setStale(false);
  }
  cout << "SOAR EVENT }\n";
  pthread_mutex_unlock(Sorts::mutex);
}

// the function that is executed by a separate thread to
// run the Soar agent
void* RunSoar(void* ptr) {
  /* For some reason if this delay isn't here, this thread will
   * grab the CPU and never let it go
   */
  sleep(1);
  ((sml::Kernel*) ptr)->RunAllAgentsForever(sml::sml_INTERLEAVE_DECISION);

  // just to keep the compiler from warning
  return NULL;
}

void* RunOrts(void* ptr) {
  GameStateModule* gsm = (GameStateModule*) ptr;
  while(1) {
  /*
    if (!gsm->recv_view()) {
      SDL_Delay(1);
    }
  */
  gsm->recv_view();
  //sleep(1);
  }
}

typedef SimpleTerrain::ST_Terrain Terrain;

int main(int argc, char *argv[]) {

  int port = 7777;
  string host = "127.0.0.1";
  char* productions = NULL;
  for(int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-port") == 0) {
      port = atoi(argv[i+1]);
    }
    else if (strcmp(argv[i], "-host") == 0) {
      host = argv[i+1];
    }
    else if (strcmp(argv[i], "-productions") == 0) {
      productions = argv[i+1];
    }
  }
  

  /************************************
   * Create mutexes for action queues *
   ************************************/
  pthread_mutex_t objectActionMutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_t attentionActionMutex = PTHREAD_MUTEX_INITIALIZER;
  
  pthread_mutex_t soarMutex = PTHREAD_MUTEX_INITIALIZER;
 
  pthread_mutex_t sortsMutex = PTHREAD_MUTEX_INITIALIZER;

  /*******************
   * Init sml client *
   *******************/
  sml::Kernel* pKernel = sml::Kernel::CreateKernelInNewThread("SoarKernelSML") ;

  // Check that nothing went wrong.  We will always get back a kernel object
  // even if something went wrong and we have to abort.
  if (pKernel->HadError()) {
    cout << pKernel->GetLastErrorDescription() << endl;
    return 1;
  }

  // Create a Soar agent named test
  // NOTE: We don't delete the agent pointer.  It's owned by the kernel
  sml::Agent* pAgent = pKernel->CreateAgent("orts_agent") ;

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
    cout << "Specify some productions to load!" << endl;
    exit(1);
  }

  if (pAgent->HadError()) {
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
  GameStateModule gsm(gsmo);

  Terrain timp;
  TerrainModule tm(gsm, timp);
  gsm.add_handler(&tm);
  
  
  // connect to ORTS server
  if (!gsm.connect()) exit(10);
  cout << "connected" << endl;
  

  Game& game = gsm.get_game();
  const Map<GameTile>& m = game.get_map();
  int gridSizeX = m.get_width() / 2;
  int gridSizeY = m.get_height() / 2;


  cout <<"calling..\n";
  
  SoarInterface soarInterface( pAgent,
                               &objectActionMutex,
                               &attentionActionMutex,
                               &soarMutex);

  // map manager, using the grid tile grouping method
  GridMapTileGrouper 
    tileGrouper(game.get_map(), game.get_tile_points(), gridSizeX, gridSizeY);
  MapManager mapManager
    ( game.get_map(), game.get_tile_points(), tileGrouper);
 
  FeatureMapManager featureMapManager;
  GroupManager gm;
  OrtsInterface ortsInterface(&gsm);

  Satellite satellite;

  Sorts sorts(&soarInterface, 
              &ortsInterface, 
              &gm, 
              &mapManager, 
              &featureMapManager,
              &tm,
              &satellite,
              &sortsMutex);

  
  soarInterface.setSorts(&sorts);
  ortsInterface.setSorts(&sorts);
  gm.setSorts(&sorts);
  mapManager.setSorts(&sorts);
  featureMapManager.setSorts(&sorts);
  satellite.init();
  
  // must be connected to orts server by now
  // must initialize the gm before soar input link
  // gm intialize gets params from orts server via ortsinterface
  // and calculates the initial vision params (window size, etc.)
  // these params are then put on the input link when it is initialized
  gm.initialize();
  soarInterface.initSoarInputLink();
  
// register for all events
  gsm.add_handler(&ortsInterface);

  //  pAgent->RegisterForPrintEvent(sml::smlEVENT_PRINT, printOutput, 0);
  pKernel->RegisterForUpdateEvent(sml::smlEVENT_AFTER_ALL_OUTPUT_PHASES, SoarUpdateEventHandler, &sorts);
  //pKernel->RegisterForSystemEvent(smlEVENT_SYSTEM_START, SoarSystemEventHandler, &state);
  //pKernel->RegisterForSystemEvent(smlEVENT_SYSTEM_STOP, SoarSystemEventHandler, &state);

  // start Soar in a different thread
  pthread_attr_t soarThreadAttribs;
  pthread_attr_init(&soarThreadAttribs);
  pthread_t soarThread;
  pthread_create(&soarThread, &soarThreadAttribs, RunSoar, (void*) pKernel);
  cout << "Soar is running" << endl;


  // this drives the orts interrupt, which drives the middleware
  pthread_attr_t ortsThreadAttribs;
  pthread_attr_init(&ortsThreadAttribs);
  pthread_t ortsThread;
  pthread_create(&ortsThread, &ortsThreadAttribs, RunOrts, (void*) &gsm);
  cout << "ORTS client is running" << endl;

  pthread_join(soarThread, NULL);
  pthread_join(ortsThread, NULL);

  cout << "!!!! Everything is finished !!!!" << endl;

  return 0;
}
