#include<iostream>
#include<pthread.h>

#include "sml_Client.h"

// our includes
#include "SoarInterface.h"
#include "OrtsInterface.h"

using namespace std;

void SoarUpdateEventHandler(sml::smlUpdateEventId id, 
                            void *pUserData,
                            sml::Kernel *pKernel,
                            sml::smlRunFlags runFlags)
{
  SoarInterface* soarInterface = (SoarInterface*) pUserData;
  soarInterface->getNewActions();
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

  SoarInterface soarInterface( pAgent
                               objectActionMutex,
                               attentionActionMutex,
                               groupActionMutex );

  // register for all events
  pAgent->RegisterForPrintEvent(smlEVENT_PRINT, printOutput, 0);
  pKernel->RegisterForUpdateEvent(smlEVENT_AFTER_ALL_OUTPUT_PHASES, SoarUpdateEventHandler, &soarInterface);
  //pKernel->RegisterForSystemEvent(smlEVENT_SYSTEM_START, SoarSystemEventHandler, &state);
  //pKernel->RegisterForSystemEvent(smlEVENT_SYSTEM_STOP, SoarSystemEventHandler, &state);
  
  /*********************************
   * Set up the connection to ORTS *
   *********************************/
  GameStateModule::Options::add();

  GameStateModule::Options gsmo;
  GameStateModule gsm(gsmo);

  OrtsInterface ortsInterface(&gsm, &soarInterface);
  gsm.add_handler(&ortsInterface);

  // connect to ORTS server
  if (!gsm.connect()) exit(10);
  cout << "connected" << endl;

  // start Soar
  pKernel->RunAllAgentsForever();
  cout << "Soar is running" << endl;

  // this drives the orts interrupt, which drives the middleware
  while(!gsm.recv_view()) {
    SDL_Delay(1);
  }
}
