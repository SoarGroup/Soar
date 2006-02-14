#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

#include "sml_Client.h"

// our includes
#include "SoarInterface.h"
#include "OrtsInterface.h"

using namespace std;

void printOutput(sml::smlPrintEventId id,
                 void*                pUserData,
                 sml::Agent*          pAgent,
                 const char*          pMessage)
{
  cout << "[SOAR] " << pMessage << endl;
}

void SoarUpdateEventHandler(sml::smlUpdateEventId id, 
                            void*                 pUserData,
                            sml::Kernel*          pKernel,
                            sml::smlRunFlags      runFlags)
{
//  cout << "Soar Event triggered" << endl;
  sml::Agent *agent = pKernel->GetAgent("orts_agent");
  SoarInterface* soarInterface = (SoarInterface*) pUserData;
  soarInterface->getNewSoarOutput();

  cout << "### COMMIT ABOUT TO BE CALLED ###" << endl;
  agent->Commit();
  cout << "### COMMIT FINISHED ###" << endl;
//  cout << "Soar Event processing finished" << endl;
}

// the function that is executed by a separate thread to
// run the Soar agent
void* RunSoar(void* ptr) {
  /* For some reason if this delay isn't here, this thread will
   * grab the CPU and never let it go
   */
  sleep(1);
  ((sml::Kernel*) ptr)->RunAllAgentsForever();

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
  pthread_mutex_t groupActionMutex = PTHREAD_MUTEX_INITIALIZER;

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

  GameStateModule::Options gsmo;
  gsmo.port = port;
  gsmo.host = host;
  GameStateModule gsm(gsmo);

  SoarInterface soarInterface( pAgent,
                               &gsm,
                               &objectActionMutex,
                               &attentionActionMutex,
                               &groupActionMutex );

  // register for all events
  pAgent->RegisterForPrintEvent(sml::smlEVENT_PRINT, printOutput, 0);
  pKernel->RegisterForUpdateEvent(sml::smlEVENT_AFTER_ALL_OUTPUT_PHASES, SoarUpdateEventHandler, &soarInterface);
  //pKernel->RegisterForSystemEvent(smlEVENT_SYSTEM_START, SoarSystemEventHandler, &state);
  //pKernel->RegisterForSystemEvent(smlEVENT_SYSTEM_STOP, SoarSystemEventHandler, &state);
  

  // instantiate the group manager
  GroupManager gm(&soarInterface);

  OrtsInterface ortsInterface(&gsm, &soarInterface, &gm);
  gm.setORTSIO(&ortsInterface);
  gsm.add_handler(&ortsInterface);

  // connect to ORTS server
  if (!gsm.connect()) exit(10);
  cout << "connected" << endl;

  // can't do these in the constructor, have to wait until connected to server
  ortsInterface.setMyPid(gsm.get_game().get_client_player());
  soarInterface.initSoarInputLink();

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
