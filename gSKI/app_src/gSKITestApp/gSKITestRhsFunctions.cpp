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

#include "IgSKI_Kernel.h"
#include "IgSKI_AgentManager.h"
#include "IgSKI_Agent.h"
#include "IgSKI_ProductionManager.h"
#include "gSKI_Stub.h"

void testRhsFunctions(const std::string& prodFileName)
{
   CreateValue cv;
   ConcatNumbers  cn;
   VerifyResult vr;

      //
   // Create a Kernel Factory
   gSKI::IKernelFactory* kF = gSKI_CreateKernelFactory();

   gSKI::IKernel* k        = kF->Create();
   MegaAssert(k != 0, "Cannot create kernel");

   gSKI::IAgentManager* am = k->GetAgentManager();
   MegaAssert(am != 0, "Cannot get agent manager.");

   gSKI::IAgent* a         = am->AddAgent("TestRHSFunctions");
   MegaAssert(am != 0, "Cannot create agent");
   
   DoHalt       dh(a);

   a->AddClientRhsFunction(&cv);
   a->AddClientRhsFunction(&cn);
   a->AddClientRhsFunction(&vr);
   a->AddClientRhsFunction(&dh);
   
   gSKI::IProductionManager* pm = a->GetProductionManager();
   MegaAssert(pm != 0, "Could not obtain the production manager.");
   
   bool loadFileSucceed = pm->LoadSoarFile(prodFileName.c_str());
   MegaAssert(loadFileSucceed, "Could not load rhs function test productions.");

   gSKI::tIProductionIterator* it = pm->GetAllProductions();
   int nProds = it->GetNumElements();
   MegaAssert(nProds == 6, "Not all productions loaded.");

   a->RunInClientThread(gSKI_RUN_FOREVER);

   a->RemoveClientRhsFunction(cv.GetName());
   a->RemoveAllClientRhsFunctions();
}


