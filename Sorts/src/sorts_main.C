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
#include<fstream>
#include<time.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>
#include<sched.h>

#include <SDL/SDL.h>

#include <sml_Client.h>

// our includes
#include "SoarInterface.h"
#include "OrtsInterface.h"
#include "AttackManagerRegistry.h"
#include "MineManager.h"
#include "FeatureMapManager.h"
#include "GameActionManager.h"
#include "Sorts.h"

#include "TerrainModule.H"
#include "Demo_SimpleTerrain.H"

#define msg std::cout << "MAIN: "

#define MAX_SOAR_AHEAD_CYCLES 5

using namespace sml;
using std::string;
using std::ofstream;

bool useSoarStops;
bool soarHalted;
bool useRL;

string fileWrite
( smlRhsEventId id, 
  void*         pUserData, 
  Agent*        pAgent,
  char const*   pFunctionName, 
  char const*   pArgument )
{
  bool append = false;
  string arg = "";
  string output = "";
  int outputStart = 0;
  ofstream f;
  for(int i = 0; pArgument[i] != 0; ++i) {
    if (pArgument[i] == ' ') {
      if (arg == "-a") {
        append = true;
        arg = "";
      }
      else {
        outputStart = i + 1;
        if (append) {
          f.open(arg.c_str(), ios::out | ios::app);
        }
        else {
          f.open(arg.c_str(), ios::out);
        }
        break;
      }
    }
    else {
      arg += pArgument[i];
    }
  }

  if (!f.is_open()) {
    return "Cannot open file";
  }
  for(int i = outputStart; pArgument[i] != 0; ++i) {
    f << pArgument[i];
  }
  f.close();
  return "";
}

string RLHalt
( smlRhsEventId id, 
  void*         pUserData, 
  Agent*        pAgent,
  char const*   pFunctionName, 
  char const*   pArgument )
{
  cout << "RUN HAS ENDED" << endl;
  pAgent->ExecuteCommandLine("command-to-file rl-rules print --RL --full");
  pAgent->ExecuteCommandLine("command-to-file rl-scores print --RL");
  
  *((bool*) pUserData) = true; // set soarHalted to true
}

void printOutput
( smlPrintEventId id,
  void*           pUserData, 
  Agent*          pAgent,
  const char*     pMessage)
{
  ofstream& file = *((ofstream*) pUserData);
  file << time(NULL) << "| " << pMessage << endl;
  //file << pMessage << endl;

#ifdef USE_CANVAS
  // enable this block to print the soar decision in the canvas,
  // which hurts performance a bit
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

void SoarStartRunEventHandler
( smlRunEventId id, 
  void*         pUserData, 
  Agent*        agent, 
  smlPhase      phase ) 
{
  pthread_mutex_unlock(Sorts::mutex);
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
  static int cycles = 0;

  unsigned long st = gettime();
  if (Sorts::cyclesSoarAhead > 5) {
    Sorts::SoarIO->stopSoar();
  }
  Sorts::cyclesSoarAhead++;

  cout << "SOAR TRYING TO LOCK MUTEX" << endl;
  pthread_mutex_lock(Sorts::mutex);
  cout << "SOAR EVENT {\n";
  
  if (Sorts::catchup == true) {
    msg << "ignoring Soar event, ORTS is behind.\n";
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
    agent->Commit();
    Sorts::SoarIO->setStale(false);
  }
  cout << "SOAR EVENT }" << endl;
 // msg << "TIME " << (gettime() - st) / 1000 << endl;
  pthread_mutex_unlock(Sorts::mutex);
  
  ++cycles;
//  if (cycles == 150) {
//    agent->ExecuteCommandLine("exit");
//  }
}

// the function that is executed by a separate thread to
// run the Soar agent
void* RunSoar(void* ptr) {
  /* For some reason if this delay isn't here, this thread will
   * grab the CPU and never let it go
   */
  sleep(3);

  if (useSoarStops) {
    while (!soarHalted) {
      pthread_mutex_lock(Sorts::mutex);
      ((Agent*) ptr)->Commit();
      ((Agent*) ptr)->RunSelfForever();
      // spin until Soar gets started again
      while (!Sorts::SoarIO->isSoarRunning()) {
        sched_yield();
        usleep(60000); // assuming 8 fps from server
      }
    }
  }
  else {
    pthread_mutex_lock(Sorts::mutex);
    // to be unlocked upon the start event
    ((Agent*) ptr)->Commit();
    cout << "SOAR GOES IN" << endl;
    ((Agent*) ptr)->RunSelfForever();
    cout << "SOAR GOES OUT" << endl;
  }
}

void* RunOrts(void* ptr) {
  GameStateModule* gsm = (GameStateModule*) ptr;

  while(!soarHalted) {
    if (!gsm->recv_view()) {
      sched_yield();
      //usleep(30000);
    }
  }
}

typedef Demo_SimpleTerrain::ST_Terrain Terrain;

Agent* initSoarRemote() {
   Kernel* pKernel = Kernel::CreateRemoteConnection(true, "127.0.0.1", 12121);

  if (!pKernel) {
    cout << "Can't connect to remote kernel" << endl;
    return NULL;
  }

  pKernel->AddRhsFunction("filewrite", &fileWrite, NULL);
  pKernel->AddRhsFunction("rlhalt", &RLHalt, &soarHalted);

  // Check that nothing went wrong.  We will always get back a kernel object
  // even if something went wrong and we have to abort.
  if (pKernel->HadError()) {
    cout << pKernel->GetLastErrorDescription() << std::endl;
    return NULL;
  }

  // Create a Soar agent named test
  // NOTE: We don't delete the agent pointer.  It's owned by the kernel
//  Agent* pAgent = pKernel->CreateAgent("orts_agent") ;
  Agent* pAgent = pKernel->GetAgentByIndex(0);

  if (!pAgent) {
    cout << "Remote kernel doesn't have an agent" << endl;
    return NULL;
  }

  // Check that nothing went wrong
  // NOTE: No agent gets created if thereâs a problem, so we have to check foor
  // errors through the kernel object.
  if (pKernel->HadError()) {
    cout << pKernel->GetLastErrorDescription() << std::endl ;
    return NULL;
  }

  pKernel->SetAutoCommit(false);

  return pAgent;
}

Agent* initSoarLocal(int soarPort, char* productions) {

  Kernel* pKernel = Kernel::CreateKernelInCurrentThread("SoarKernelSML", soarPort) ;
  std::cout << "done\n";

  pKernel->AddRhsFunction("filewrite", &fileWrite, NULL);
  pKernel->AddRhsFunction("rlhalt", &RLHalt, &soarHalted);

  // Check that nothing went wrong.  We will always get back a kernel object
  // even if something went wrong and we have to abort.
  if (pKernel->HadError()) {
    std::cout << pKernel->GetLastErrorDescription() << std::endl;
    return NULL;
  }

  // Create a Soar agent named test
  // NOTE: We don't delete the agent pointer.  It's owned by the kernel
  Agent* pAgent = pKernel->CreateAgent("sorts") ;

  // Check that nothing went wrong
  // NOTE: No agent gets created if thereâs a problem, so we have to check foor
  // errors through the kernel object.
  if (pKernel->HadError()) {
    std::cout << pKernel->GetLastErrorDescription() << std::endl ;
    return NULL;
  }

  // Load some productions
  pAgent->LoadProductions(productions) ;

  if (pAgent->HadError()) {
    cout << "Soar agent reported an error:\n";
    cout << pAgent->GetLastErrorDescription() << endl ;
    return NULL;
  }

  pKernel->SetAutoCommit(false);

  return pAgent;
}

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
  bool remote_kernel = false;
  bool noSortsCanvas = false;
  bool oldAgent = false;

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
    else if (strcmp(argv[i], "-remote-kernel") == 0) {
      remote_kernel = true;
    }
    else if (strcmp(argv[i], "-no-canvas") == 0) {
      noSortsCanvas = true;
    }
    else if (strcmp(argv[i], "-old-agent") == 0) {
      oldAgent = true;
    }
  }
  
  srand(seed);
  
  pthread_mutex_t sortsMutex = PTHREAD_MUTEX_INITIALIZER;

  // Load some productions
  if (productions == NULL) {
    cout << "ERROR: No productions specified. Use -p or -productions.\n";
    exit(1);
  }

  Agent* pAgent;

  if (remote_kernel) {
    pAgent = initSoarRemote();
  }
  else {
    pAgent = initSoarLocal(soarPort, productions);
  }

  if (!pAgent) {
    cout << "Could not start the Soar agent" << endl;
    exit(1);
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
  
  // connect to ORTS server
  if (!gsm.connect()) exit(10);
  msg << "connected to ORTS server" << std::endl;
  
  SoarInterface soarInterface(pAgent, oldAgent);

  FeatureMapManager featureMapManager;
  PerceptualGroupManager pgm;
  OrtsInterface ortsInterface(&gsm);

  GameActionManager gaMan;
  SpatialDB spatialDB;
  MineManager mineMan;
  AttackManagerRegistry amr;

  Sorts sorts(&soarInterface, 
              &ortsInterface, 
              &pgm, 
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

  double xDim = ortsInterface.getMapXDim();
  double yDim = ortsInterface.getMapYDim();
  if (not noSortsCanvas) {
    // initialize the canvas
    Sorts::canvas.init
      ( xDim, 
        yDim,
        1.2 );
  }

  list<pair<double, double> > worldBounds;
  worldBounds.push_back(make_pair((double)10,(double)10));
  worldBounds.push_back(make_pair(xDim-10,(double)10));
  worldBounds.push_back(make_pair(xDim-10,yDim-10));
  worldBounds.push_back(make_pair((double)10,yDim-10));

// register for all events
  gsm.add_handler(&ortsInterface);

  pAgent->RegisterForRunEvent(smlEVENT_BEFORE_RUN_STARTS, SoarStartRunEventHandler, NULL);
  pAgent->RegisterForRunEvent(smlEVENT_AFTER_OUTPUT_PHASE, SoarOutputEventHandler, NULL);
  pAgent->RegisterForRunEvent(smlEVENT_AFTER_DECISION_CYCLE, SoarAfterDecisionCycleEventHandler, NULL);
 // pAgent->RegisterForRunEvent(smlEVENT_BEFORE_INPUT_PHASE, SoarInputEventHandler, &sorts);

  ofstream* soar_log = 0;
  if (printSoar) {
    soar_log = new ofstream("soar.log", ios::out);
    if (soar_log && soar_log->is_open()) {
      pAgent->RegisterForPrintEvent(smlEVENT_PRINT, printOutput, soar_log);  
    }
    else {
      cout << "Failed to open soar.log for writing." << endl;
      soar_log = 0;
    }
  }
 
  // start Soar in a different thread
  soarHalted = false;

  pthread_t soarThread;
  if (!remote_kernel) {
    pthread_create(&soarThread, NULL, RunSoar, (void*) pAgent);
    msg << "Soar is running" << std::endl;
  }

  RunOrts(&gsm);

  if (!remote_kernel) {
    pthread_join(soarThread, NULL);
    msg << "Everything finished" << endl;
  }

//  pthread_join(ortsThread, NULL);
//  msg << "ORTS client exited" << endl;
 
  if (soar_log) {
    soar_log->close();
    delete soar_log;
  }

#ifdef USE_CANVAS
  Sorts::canvas.quit();
#endif

  exit(0);
}
