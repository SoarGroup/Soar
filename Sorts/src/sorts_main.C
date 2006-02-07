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
  //cout << "[SOAR] " << pMessage << endl;
}

void SoarUpdateEventHandler(sml::smlUpdateEventId id, 
                            void*                 pUserData,
                            sml::Kernel*          pKernel,
                            sml::smlRunFlags      runFlags)
{
  /*
  cout << "Soar Event triggered" << endl;
  SoarInterface* soarInterface = (SoarInterface*) pUserData;
  soarInterface->getNewSoarOutput();
  cout << "Soar Event processing finished" << endl;
  */
}

// the function that is executed by a separate thread to
// run the Soar agent
void* RunSoar(void* ptr) {
  ((sml::Kernel*) ptr)->RunAllAgentsForever();
}

void* RunOrts(void* ptr) {
  GameStateModule* gsm = (GameStateModule*) ptr;
  while(1) {
    if (!gsm->recv_view()) {
      SDL_Delay(1);
    }
  }
}

int main(int argc, char *argv[]) {

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
  sml::Agent* pAgent = pKernel->CreateAgent("test") ;

  // Check that nothing went wrong
  // NOTE: No agent gets created if thereâs a problem, so we have to check foor
  // errors through the kernel object.
  if (pKernel->HadError()) {
    std::cout << pKernel->GetLastErrorDescription() << std::endl ;
    return 1;
  }

  // Load some productions
  pAgent->LoadProductions(argv[1]) ;

  if (pAgent->HadError()) {
    cout << pAgent->GetLastErrorDescription() << endl ;
    return 1;
  }

  SoarInterface soarInterface( pAgent,
                               &objectActionMutex,
                               &attentionActionMutex,
                               &groupActionMutex );

  // register for all events
//  pAgent->RegisterForPrintEvent(sml::smlEVENT_PRINT, printOutput, 0);
//  pKernel->RegisterForUpdateEvent(sml::smlEVENT_AFTER_ALL_OUTPUT_PHASES, SoarUpdateEventHandler, &soarInterface);
  //pKernel->RegisterForSystemEvent(smlEVENT_SYSTEM_START, SoarSystemEventHandler, &state);
  //pKernel->RegisterForSystemEvent(smlEVENT_SYSTEM_STOP, SoarSystemEventHandler, &state);
  

  // instantiate the group manager
  GroupManager gm(&soarInterface);

  /*********************************
   * Set up the connection to ORTS *
   *********************************/
  GameStateModule::Options::add();

  GameStateModule::Options gsmo;
  GameStateModule gsm(gsmo);


  OrtsInterface ortsInterface(&gsm, &soarInterface, &gm);
  gsm.add_handler(&ortsInterface);

  // connect to ORTS server
  if (!gsm.connect()) exit(10);
  cout << "connected" << endl;

  // start Soar in a different thread
  pthread_t soarThread;
  pthread_create(&soarThread, NULL, RunSoar, (void*) pKernel);
  cout << "Soar is running" << endl;

  // this drives the orts interrupt, which drives the middleware
  pthread_t ortsThread;
  pthread_create(&ortsThread, NULL, RunOrts, (void*) &gsm);
  cout << "ORTS client is running" << endl;

  pthread_join(soarThread, NULL);
  pthread_join(ortsThread, NULL);

  cout << "!!!! Everything is finished !!!!" << endl;

  return 0;
}
