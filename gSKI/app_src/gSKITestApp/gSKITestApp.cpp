/********************************************************************
* @file gskitestapp.cpp 
*********************************************************************
* @remarks Copyright (C) 2002 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   6/10/2002   15:02
*
* purpose: 
*********************************************************************/

#include "gSKITestRhsFunctions.h"

#include "gSKI.h"
#include <iostream>
#include <fstream>

#include "IgSKI_KernelFactory.h"
#include "gSKI_Stub.h"
#include "IgSKI_Kernel.h"
#include "gSKI_ErrorIds.h"
#include "IgSKI_AgentManager.h"
#include "IgSKI_Agent.h"
#include "IgSKI_ProductionManager.h"
#include "IgSKI_OutputProcessor.h"
#include "IgSKI_InputProducer.h"
#include "ExportLibFiles.h"


#include "MegaAssert.h"

// For unit testing
//#include "MegaUnitInterfaces.h"
//#include "MegaUnitTest.h"

#include <iostream>


using namespace gSKI;
using namespace std;


/*
==================================
 ____       _       _   _   _                 _ _
|  _ \ _ __(_)_ __ | |_| | | | __ _ _ __   __| | | ___ _ __
| |_) | '__| | '_ \| __| |_| |/ _` | '_ \ / _` | |/ _ \ '__|
|  __/| |  | | | | | |_|  _  | (_| | | | | (_| | |  __/ |
|_|   |_|  |_|_| |_|\__|_| |_|\__,_|_| |_|\__,_|_|\___|_|
==================================
*/
class PrintHandler: public IPrintListener
{
public:
   PrintHandler(std::ostream* os): m_os(os) {}
   virtual void HandleEvent(egSKIEventId eventId, IAgent* agentPtr, const char* msg)
   {
      //if(msg[0] != '\n')
      //   std::cout << agentPtr->GetName() << ": ";
      
      *(m_os) << msg;
   }

private:

   std::ostream* m_os;
};
  
/*
==================================

==================================
*/
class PrintRunTrace: public IRunListener
{
public:

   PrintRunTrace(std::ostream* os): m_os(os) {}
   virtual void HandleEvent(egSKIEventId eventId, IAgent* agentPtr, egSKIPhaseType phase)
   {
      *(m_os)  << std::endl
               << agentPtr->GetName()
               << ", "
               << static_cast<unsigned int>(eventId) 
               << ": ("
               << agentPtr->GetNumSmallestStepsExecuted()
               << ", "
               << agentPtr->GetNumPhasesExecuted()
               << ", "
               << agentPtr->GetNumDecisionCyclesExecuted()
               << ", "
               << agentPtr->GetNumOutputsExecuted()
               << ") ";

      switch (phase)
      {
      case gSKI_INPUT_PHASE:
         *(m_os) << "--- Input Phase ---";
         break;
      case gSKI_PROPOSAL_PHASE:
         *(m_os) << "--- Proposal Phase ---";
         break;
      case gSKI_DECISION_PHASE:
         *(m_os) << "--- Decision Phase ---";
         break;
      case gSKI_APPLY_PHASE:
         *(m_os) << "--- Apply Phase ---";
         break;
      case gSKI_OUTPUT_PHASE:
         *(m_os) << "--- Output Phase ---";
         break;
      default:
         *(m_os) << "--- UNKNOWN PHASE ---";
         break;
      }
   }

private:

   std::ostream* m_os;
};

/*
==================================

==================================
*/
class BreakRunForever: public IRunListener
{
public:

   BreakRunForever(unsigned long steps, bool haltBreak = false): m_count(0), m_max(steps), m_halt(haltBreak) {}
   virtual void HandleEvent(egSKIEventId eventId, IAgent* agentPtr, egSKIPhaseType phase)
   {
      if(eventId == gSKIEVENT_AFTER_DECISION_CYCLE)
      {
         Error err;

         ++m_count;
         if(m_count == m_max)
         {
            if(!m_halt)
            {
               agentPtr->Interrupt(gSKI_STOP_AFTER_SMALLEST_STEP, gSKI_STOP_BY_RETURNING, &err);
               MegaAssert(err.Id == gSKIERR_NONE, "Error trying to interrupt _agent.");
            }
            else
            {
               agentPtr->Halt(&err);
               MegaAssert(err.Id == gSKIERR_NONE, "Error trying to halt _agent.");
            }
         }
      }
   }
private:
   unsigned long m_count; /**< Count used to break run forever after a certain number of steps */
   unsigned long m_max;
   bool          m_halt;
};

/*
==================================

==================================
*/
class BreakRunForeverAll: public IRunListener
{
public:

   BreakRunForeverAll(IAgentManager* am, unsigned long steps): m_count(0), m_max(steps), m_am(am) {}
   virtual void HandleEvent(egSKIEventId eventId, IAgent* agentPtr, egSKIPhaseType phase)
   {
      if(eventId == gSKIEVENT_AFTER_DECISION_CYCLE)
      {
         Error err;

         ++m_count;
         if(m_count == m_max)
         {
            m_am->HaltAll(&err);
            MegaAssert(err.Id == gSKIERR_NONE, "Error trying to halt _agent.");
         }
      }
   }
private:
   unsigned long  m_count; /**< Count used to break run forever after a certain number of steps */
   unsigned long  m_max;
   IAgentManager* m_am;
};

/*
==================================
    _       _     _ ____
   / \   __| | __| |  _ \ ___ _ __ ___   _____   _____
  / _ \ / _` |/ _` | |_) / _ \ '_ ` _ \ / _ \ \ / / _ \
 / ___ \ (_| | (_| |  _ <  __/ | | | | | (_) \ V /  __/
/_/ _ \_\__,_|\__,_|_| \_\___|_| |_| |_|\___/ \_/ \___|
   / \   __ _  ___ _ __ | |_| |   (_)___| |_ ___ _ __   ___ _ __
  / _ \ / _` |/ _ \ '_ \| __| |   | / __| __/ _ \ '_ \ / _ \ '__|
 / ___ \ (_| |  __/ | | | |_| |___| \__ \ ||  __/ | | |  __/ |
/_/   \_\__, |\___|_| |_|\__|_____|_|___/\__\___|_| |_|\___|_|
        |___/
==================================
*/
class AddRemoveAgentListener: public IRunListener
{
public:

   AddRemoveAgentListener(IAgentManager* am, bool remove): m_remove(remove), m_am(am), m_acted(false) {}
   virtual void HandleEvent(egSKIEventId eventId, IAgent* agentPtr, egSKIPhaseType phase)
   {
      Error err;
      if(m_remove)
      {
         m_am->RemoveAgent(agentPtr, &err);
         MegaAssert(err.Id == gSKIERR_NONE, "Had problem removing _agent from callback");
      }
      else if(!m_acted)
      {
         m_acted = true;
         m_am->AddAgent("MultiAgentTest4", "../lib_src/SKx/bin/TestProductions/waterjug2.soar", 
                          false, gSKI_O_SUPPORT_MODE_3, &err);
         MegaAssert(err.Id == gSKIERR_NONE, "Had problem adding _agent from callback.");

      }
   }

private:
   bool           m_remove;       /**< Count used to break run forever after a certain number of steps */
   IAgentManager* m_am;
   bool           m_acted;
};

/*
==================================
 ____                _            _   _             ____       _       _
|  _ \ _ __ ___   __| |_   _  ___| |_(_) ___  _ __ |  _ \ _ __(_)_ __ | |_ ___ _ __
| |_) | '__/ _ \ / _` | | | |/ __| __| |/ _ \| '_ \| |_) | '__| | '_ \| __/ _ \ '__|
|  __/| | | (_) | (_| | |_| | (__| |_| | (_) | | | |  __/| |  | | | | | ||  __/ |
|_|   |_|  \___/ \__,_|\__,_|\___|\__|_|\___/|_| |_|_|   |_|  |_|_| |_|\__\___|_|
==================================
*/
class ProductionPrinter: public IProductionListener
{
public:
   ProductionPrinter(std::ostream* os, bool traceProductions = true): 
      m_os(os), m_traceProductions(traceProductions) {}
   virtual void HandleEvent(egSKIEventId eventId, IAgent* agentPtr, 
                            IProduction* prod, IProductionInstance* prodInstance)
   {
      if(m_traceProductions)
      {
         if(eventId == gSKIEVENT_AFTER_PRODUCTION_ADDED)
            *(m_os) << std::endl << "* Production Added: " << prod->GetName();
         else if(eventId == gSKIEVENT_BEFORE_PRODUCTION_REMOVED)
            *(m_os) << std::endl << "# Production Removed: " << prod->GetName();
         else if(eventId == gSKIEVENT_AFTER_PRODUCTION_FIRED)
            *(m_os) << std::endl << "! Production Fired: " << prod->GetName();
      }
   }
private:
   std::ostream* m_os;
   bool          m_traceProductions;
};

/*
==================================
 _                _   _                 _ _
| |    ___   __ _| | | | __ _ _ __   __| | | ___ _ __
| |   / _ \ / _` | |_| |/ _` | '_ \ / _` | |/ _ \ '__|
| |__| (_) | (_| |  _  | (_| | | | | (_| | |  __/ |
|_____\___/ \__, |_| |_|\__,_|_| |_|\__,_|_|\___|_|
            |___/
==================================
*/
class LogHandler: public ILogListener
{
public:
   LogHandler(std::ostream* os): m_os(os) {}
   virtual void HandleEvent(egSKIEventId eventId, IKernel *kernelPtr, const char *msg)
   {
      if(msg[0] != '\n')
         *(m_os) << "Log message: " << ": ";
      
      *(m_os) << msg;
   }
private:

   std::ostream* m_os;
};

/*
==================================
==================================
*/
class InputProducer: public IInputProducer
{
public:
  InputProducer():m_count(0),m_wme(0) {}
  virtual ~InputProducer() {}

  virtual void Update(IWorkingMemory* wmemory,
                      IWMObject* obj)
  {
    m_count++;

    //std::cout << "Updating input link!" << std::endl;

     if ( m_wme != 0 ) {
       wmemory->RemoveWme(m_wme);
       m_wme->Release();
     }

     m_wme = wmemory->AddWmeInt(obj, 
				"cycle-count", 
				wmemory->GetAgent()->GetNumDecisionCyclesExecuted());
  }

private:
  int m_count;
  IWme* m_wme;
};

/*
==================================
  ___        _               _   ____
 / _ \ _   _| |_ _ __  _   _| |_|  _ \ _ __ ___   ___ ___ ___ ___  ___  _ __
| | | | | | | __| '_ \| | | | __| |_) | '__/ _ \ / __/ _ Y __/ __|/ _ \| '__|
| |_| | |_| | |_| |_) | |_| | |_|  __/| | | (_) | (_|  __|__ \__ \ (_) | |
 \___/ \__,_|\__| .__/ \__,_|\__|_|   |_|  \___/ \___\___|___/___/\___/|_|
                |_|
==================================
*/
class OutputProcessor: public IOutputProcessor
{
public:
   OutputProcessor(std::ostream* os): m_os(os) {}

   virtual void ProcessOutput(IWorkingMemory* workingMemory,
                              IWMObject* obj)
   {
     // Getting the value wme 
     tIWmeIterator* it = obj->GetWMEs("value");
     
     for ( ; it->IsValid(); it->Next() ) {

       IWme* wme = it->GetVal();
       int value = wme->GetValue()->GetInt();
         
       *(m_os) 
          << std::endl
          << "Processing count object with value = " << value 
          << std::endl;
     }

     // Marking the output as processed
     workingMemory->AddWmeString(obj, "processed", "*yes*");
   }
private:

   std::ostream* m_os;
};

/*
==================================
            _       _   ____                _            _   _
 _ __  _ __(_)_ __ | |_|  _ \ _ __ ___   __| |_   _  ___| |_(_) ___  _ __
| '_ \| '__| | '_ \| __| |_) | '__/ _ \ / _` | | | |/ __| __| |/ _ \| '_ \
| |_) | |  | | | | | |_|  __/| | | (_) | (_| | |_| | (__| |_| | (_) | | | |
| .__/|_|  |_|_| |_|\__|_|   |_|  \___/ \__,_|\__,_|\___|\__|_|\___/|_| |_|
|_|    _     _
| |   (_)___| |_
| |   | / __| __|
| |___| \__ \ |_
|_____|_|___/\__|
==================================
*/
void printProductionList(std::ostream& os, const char *msg, tIProductionIterator *prodInstances)
{

   os << endl << msg << endl << "--------------" << endl;
   while(prodInstances->IsValid()){
      const char *c = prodInstances->GetVal()->GetName();
      os << c << std::endl;
      prodInstances->GetVal()->GetText();
      prodInstances->GetVal()->Release();
      prodInstances->Next();
   }
   prodInstances->Release();

}

#ifdef _WIN32
#include "direct.h"
#endif
/*
==================================
                 _
 _ __ ___   __ _(_)_ __
| '_ ` _ \ / _` | | '_ \
| | | | | | (_| | | | | |
|_| |_| |_|\__,_|_|_| |_|
==================================
*/
int main(int argc, char* argv[]) 
{ 
   std::ofstream   testFile("../../build/active/unittests/gSKI_TestOutput.txt");

   PrintHandler      printHandler(&std::cout);
   LogHandler        logHandler(&testFile);
   OutputProcessor   outputProcessor(&std::cout);
   ProductionPrinter prodListener(&testFile, true);
   ProductionPrinter prodListener2(&std::cout, false);

#ifndef NO_MEGA_UNIT_TESTS
   {
      using namespace MegaUnit;

      // Run unit tests first
      const char* failFile = "UnitFailures.txt";
      const char* detFile  = "UnitDetails.txt";
      
      IUnitTestListener* mu_listener = GetDefaultLogger(failFile, detFile);

      GetTestManager()->AddTestListener(mu_listener);
      GetTestManager()->RunTests();
      GetTestManager()->RemoveTestListener(mu_listener);
   }
#endif

   //
   // Create a Kernel Factory
   IKernelFactory* kF = gSKI_CreateKernelFactory();
   
   //
   // Create some test Kernels
   IKernel* k = kF->Create();
   IKernel* ka = kF->Create();
   kF->DestroyKernel(ka);
   IKernel* kb = kF->Create();
   IKernel* kc = kF->Create();

   //
   // Set the logging level for the kernel.
   k->SetLogActivityLevel(gSKI_LOG_ALL);

   //
   // Get the list of created factories from the Kernel Factory.
   Error err;
   tIInstanceInfoIterator *instances = kF->GetInstanceIterator(&err);

   //
   // Iteratre over the list of created factories.
   while(instances->IsValid())
   {
      const char *c = instances->GetVal()->GetServerName();
      std::cout << c << std::endl;
      instances->Next();
   }

   for( ; instances->IsValid() ; instances->Next())
   {

   }
   //
   // Clean up the iterator.
   instances->Release();

   //
   // Get Version information for the Kernel Factory.
   Version gSKI_Version = kF->GetGSKIVersion();
   Version Soar_Version = kF->GetKernelVersion();

   //
   // Create an _agent manager.
   IAgentManager *IAM = k->GetAgentManager();

   k->AddLogListener(gSKIEVENT_LOG_DEBUG, &logHandler);
   k->AddLogListener(gSKIEVENT_LOG_ERROR, &logHandler);
   k->AddLogListener(gSKIEVENT_LOG_INFO, &logHandler);
   k->AddLogListener(gSKIEVENT_LOG_WARNING, &logHandler);

   //
   // Create an _agent, then try to fetch it from the _agent manager.
   IAgent *_agent = IAM->AddAgent("MyNewAgent");
   IAgent *_agent2 = IAM->GetAgent("MyNewAgent");
   MegaAssert( _agent == _agent2, "These _agent's should be the same!");

   // Testing the working memory and input and output links
   IWorkingMemory* wm = _agent->GetWorkingMemory();
   MegaAssert( wm != 0, "The working memory object should not be null!");

   IInputLink* ilink = _agent->GetInputLink();
   MegaAssert( ilink != 0, "The input link should not be null!");

   IOutputLink* olink = _agent->GetOutputLink();
   MegaAssert( olink!= 0, "The output link should not be null!");

   // Testing the Working Memory get _agent method
   MegaAssert( _agent == wm->GetAgent(), "GetAgent() returns the wrong _agent!");


   IAM->RemoveAgent(_agent);

   _agent = IAM->AddAgent("MyNewAgent");
   _agent2 = IAM->GetAgent("MyNewAgent");

   IProductionManager *IPM = _agent2->GetProductionManager();


#ifdef _WIN32
      char tmp[255];
   _getcwd(tmp, 255);
#endif

   _agent2->AddPrintListener(gSKIEVENT_PRINT, &printHandler);
   _agent2->GetOutputLink()->AddOutputProcessor("count",&outputProcessor);

   // Getting the root input object to add a listener to
   InputProducer iprod;
   IWMObject* wmobj;
   _agent2->GetInputLink()->GetRootObject(&wmobj);
   _agent2->GetInputLink()->AddInputProducer(wmobj,
                                             &iprod);

   IPM->LoadSoarFile("../lib_src/SKx/bin/TestProductions/OutputLinkTest.soar", &err);
   //MegaAssert((err.Id == gSKIERR_NONE), "Failed to load Soar File.");

   IPM->AddProductionListener(gSKIEVENT_AFTER_PRODUCTION_ADDED, &prodListener);
   IPM->AddProductionListener(gSKIEVENT_BEFORE_PRODUCTION_REMOVED, &prodListener);
   IPM->AddProductionListener(gSKIEVENT_AFTER_PRODUCTION_FIRED, &prodListener);
   IPM->AddProductionListener(gSKIEVENT_AFTER_PRODUCTION_ADDED, &prodListener2);
   IPM->AddProductionListener(gSKIEVENT_BEFORE_PRODUCTION_REMOVED, &prodListener2);
   IPM->AddProductionListener(gSKIEVENT_AFTER_PRODUCTION_FIRED, &prodListener2);

   // SOURCING ISN'T WORKING YET!!
   //IPM->LoadSoarFile("../lib_src/SKx/bin/TestProductions/simple.soar", &err);
   //IPM->LoadSoarFile("../lib_src/SKx/bin/TestProductions/selection.soar", &err);
   //IPM->LoadSoarFile("../lib_src/SKx/bin/TestProductions/waterjug2.soar", &err);
//   MegaAssert((err.Id == gSKIERR_NONE), "Failed to load Soar File.");

   //printProductionList("User Productions" ,IPM->GetAllProductions());
   //printProductionList("User Productions" ,IPM->GetUserProductions());
   //printProductionList("User Productions" ,IPM->GetDefaultProductions());
   //printProductionList("User Productions" ,IPM->GetChunks());
   //printProductionList("User Productions" ,IPM->GetJustifications());

   /////////////////////////////////////////////////////
   // Start by running a single _agent for several smallest steps

   Error runErr;
   PrintRunTrace   runTraceListener(&testFile);
   PrintRunTrace   runTraceListener2(&std::cout);
   BreakRunForever breakListener(5);

   // Add to all of the runtime events (this argues for a multi-add method)
   _agent2->AddRunListener(gSKIEVENT_AFTER_SMALLEST_STEP, &runTraceListener);
   _agent2->AddRunListener(gSKIEVENT_AFTER_SMALLEST_STEP, &runTraceListener2);

   ////////////////////////////
   // We loop and run smallest steps
   for(int i = 0; i < 10; ++i)
      _agent2->RunInClientThread(gSKI_RUN_SMALLEST_STEP, 1, &runErr);
   MegaAssert(runErr.Id == gSKIERR_NONE, "Error running _agent.");

   std::cout << std::endl << "---------------------------------" << endl;
   testFile << std::endl << "---------------------------------" << endl;

   // Now tell it to run 10 steps another way
   _agent2->RunInClientThread(gSKI_RUN_SMALLEST_STEP, 10, &runErr);
   MegaAssert(runErr.Id == gSKIERR_NONE, "Error running _agent.");

   std::cout << std::endl << "---------------------------------" << endl;
   testFile << std::endl << "---------------------------------" << endl;

   ////////////////////////////
   // We loop and run phases
   for(int i = 0; i < 10; ++i)
      _agent2->RunInClientThread(gSKI_RUN_PHASE, 1, &runErr);
   MegaAssert(runErr.Id == gSKIERR_NONE, "Error running _agent.");

   std::cout << std::endl << "---------------------------------" << endl;
   testFile << std::endl << "---------------------------------" << endl;

   // Now tell it to run 10 steps another way
   _agent2->RunInClientThread(gSKI_RUN_PHASE, 10, &runErr);
   MegaAssert(runErr.Id == gSKIERR_NONE, "Error running _agent.");

   std::cout << std::endl << "---------------------------------" << endl;
   testFile << std::endl << "---------------------------------" << endl;

   ////////////////////////////
   // We loop and run phases
   for(int i = 0; i < 10; ++i)
      _agent2->RunInClientThread(gSKI_RUN_DECISION_CYCLE, 1, &runErr);
   MegaAssert(runErr.Id == gSKIERR_NONE, "Error running _agent.");

   std::cout << std::endl << "---------------------------------" << endl;
   testFile << std::endl << "---------------------------------" << endl;

   // Now tell it to run 10 steps another way
   _agent2->RunInClientThread(gSKI_RUN_DECISION_CYCLE, 10, &runErr);
   MegaAssert(runErr.Id == gSKIERR_NONE, "Error running _agent.");

   std::cout << std::endl << "---------------------------------" << endl;
   testFile << std::endl << "---------------------------------" << endl;

   // ==============
   _agent2->AddRunListener(gSKIEVENT_AFTER_DECISION_CYCLE, &breakListener);

   ////////////////////////////
   // We loop and run phases
   egSKIRunResult runResult = gSKI_RUN_COMPLETED;
   for(int i = 0; i < 10 && runResult != gSKI_RUN_COMPLETED; ++i)
      runResult = _agent2->RunInClientThread(gSKI_RUN_UNTIL_OUTPUT, 1, &runErr);
   MegaAssert(runErr.Id == gSKIERR_NONE, "Error running _agent.");

   std::cout << std::endl << "---------------------------------" << endl;
   testFile << std::endl << "---------------------------------" << endl;

   // Now tell it to run 10 steps another way
   _agent2->ClearInterrupts();
   runResult = _agent2->RunInClientThread(gSKI_RUN_UNTIL_OUTPUT, 10, &runErr);
   MegaAssert(runErr.Id == gSKIERR_NONE, "Error running _agent.");

   std::cout << std::endl << "---------------------------------" << endl;
   testFile << std::endl << "---------------------------------" << endl;

   // Try run forever wit a break
   BreakRunForever breakListener2(50);
   _agent2->AddRunListener(gSKIEVENT_AFTER_DECISION_CYCLE, &breakListener2);

   // First time it will fail
   runResult = _agent2->RunInClientThread(gSKI_RUN_FOREVER, 1, &runErr);
   MegaAssert(runErr.Id == gSKIERR_AGENT_RUNNING, "Error running _agent.");

   // Second time it will succeed
   _agent2->ClearInterrupts();
   runResult = _agent2->RunInClientThread(gSKI_RUN_FOREVER, 1, &runErr);
   MegaAssert(runErr.Id == gSKIERR_NONE, "Error running _agent.");

   ///// CLEANUP STUFF
   IAM->RemoveAgentByName(_agent->GetName());

   // Now add two agents, initialize them and run them

   Error createErr;
   _agent  = IAM->AddAgent("MultiAgentTest1", "../lib_src/SKx/bin/TestProductions/waterjug2.soar", false, gSKI_O_SUPPORT_MODE_3, &createErr);
   MegaAssert(createErr.Id == gSKIERR_NONE, "Cannot create an _agent.");
   _agent2 = IAM->AddAgent("MultiAgentTest2", "../lib_src/SKx/bin/TestProductions/waterjug2.soar", false, gSKI_O_SUPPORT_MODE_3, &createErr);
   MegaAssert(createErr.Id == gSKIERR_NONE, "Cannot create an _agent.");

   // Add listeners
   _agent->AddRunListener(gSKIEVENT_AFTER_SMALLEST_STEP, &runTraceListener);
   _agent2->AddRunListener(gSKIEVENT_AFTER_SMALLEST_STEP, &runTraceListener);
   
   std::cout << std::endl << "---------------------------------" << endl;
   testFile << std::endl << "---------------------------------" << endl;

   IAM->AddAllAgentsToRunList();

   // Now we can run them
   // Try one step first
   for(int i = 0; i < 5; ++i)
      IAM->RunInClientThread(gSKI_RUN_SMALLEST_STEP, 1, gSKI_INTERLEAVE_SMALLEST_STEP);

   // Now try doing 5 through the interface
   IAM->RunInClientThread(gSKI_RUN_SMALLEST_STEP, 5, gSKI_INTERLEAVE_SMALLEST_STEP);

   // Now add a third _agent
   _agent  = IAM->AddAgent("MultiAgentTest3", "../lib_src/SKx/bin/TestProductions/waterjug2.soar", false, gSKI_O_SUPPORT_MODE_3, &createErr);
   MegaAssert(createErr.Id == gSKIERR_NONE, "Cannot create an _agent.");

   _agent->AddRunListener(gSKIEVENT_AFTER_SMALLEST_STEP, &runTraceListener);

   IAM->AddAgentToRunList(_agent);

   // Try to run for phases

   std::cout << std::endl << "---------------------------------" << endl;
   testFile << std::endl << "---------------------------------" << endl;

   // Now we can run them
   // Try one step first
   for(int i = 0; i < 5; ++i)
      IAM->RunInClientThread(gSKI_RUN_PHASE, 1, gSKI_INTERLEAVE_SMALLEST_STEP);

   // Now try doing 5 through the interface
   IAM->RunInClientThread(gSKI_RUN_PHASE, 5, gSKI_INTERLEAVE_SMALLEST_STEP);

   // Now add a listener that will create and _agent and destroy an _agent
   //  during a callback while running
   AddRemoveAgentListener remover(IAM, true);
   _agent2->AddRunListener(gSKIEVENT_AFTER_SMALLEST_STEP, &remover);
   AddRemoveAgentListener adder(IAM, false);
   _agent->AddRunListener(gSKIEVENT_AFTER_SMALLEST_STEP, &adder);

   std::cout << std::endl << "---------------------------------" << endl;
   testFile << std::endl << "---------------------------------" << endl;

   // Now we can run them
   // Try one step first
   for(int i = 0; i < 5; ++i)
      IAM->RunInClientThread(gSKI_RUN_DECISION_CYCLE, 1, gSKI_INTERLEAVE_SMALLEST_STEP);

   // Now try doing 5 through the interface
   IAM->RunInClientThread(gSKI_RUN_DECISION_CYCLE, 5, gSKI_INTERLEAVE_SMALLEST_STEP);

   std::cout << std::endl << "---------------------------------" << endl;
   testFile << std::endl << "---------------------------------" << endl;

   // Try different interleaving
   IAM->RunInClientThread(gSKI_RUN_DECISION_CYCLE, 5, gSKI_INTERLEAVE_PHASE);

   std::cout << std::endl << "---------------------------------" << endl;
   testFile << std::endl << "---------------------------------" << endl;
   IAM->RunInClientThread(gSKI_RUN_DECISION_CYCLE, 5, gSKI_INTERLEAVE_DECISION_CYCLE);

   // Now run forever, but break after 100 cycles (maximum)
   BreakRunForeverAll haltListener(IAM, 100);
   _agent->AddRunListener(gSKIEVENT_AFTER_DECISION_CYCLE, &haltListener);

   IAM->RunInClientThread(gSKI_RUN_FOREVER, 0, gSKI_INTERLEAVE_PHASE);
   
   k->RemoveLogListener(gSKIEVENT_LOG_DEBUG, &logHandler);
   k->RemoveLogListener(gSKIEVENT_LOG_ERROR, &logHandler);
   k->RemoveLogListener(gSKIEVENT_LOG_INFO, &logHandler);
   k->RemoveLogListener(gSKIEVENT_LOG_WARNING, &logHandler);

   kF->DestroyKernel(k);
   kF->DestroyKernel(kb);
   kF->DestroyKernel(kc);
   kF->Release();
   
   testRhsFunctions("../lib_src/SKx/bin/TestProductions/test_rhs_functions.soar");

   getchar();

   return 0;
}
