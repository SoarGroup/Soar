/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski.h 
*********************************************************************
* created:	   6/10/2002   10:46
*
* purpose: This is the primary interface file for gSKI.  This will provide
*          the abstract interface the defines gSKI.
*********************************************************************/
#ifndef GSKI_HEADER
#define GSKI_HEADER

#include "IgSKI_PerformanceManager.h"
#include "IgSKI_ProductionManager.h"
#include "IgSKI_AgentThreadGroup.h"
#include "IgSKI_AgentRunControl.h"
#include "IgSKI_WorkingMemory.h"
#include "IgSKI_KernelFactory.h"
#include "IgSKI_EventManager.h"
#include "IgSKI_InstanceInfo.h"
#include "IgSKI_Iterator.h"
#include "gSKI_Enumerations.h"
#include "IgSKI_AgentManager.h"
#include "IgSKI_ConditionSet.h"
#include "IgSKI_Production.h"
#include "IgSKI_ProductionMatch.h"
#include "IgSKI_OutputLink.h"
#include "IgSKI_OutputProcessor.h"
#include "IgSKI_Condition.h"
#include "IgSKI_ConditionSet.h"
#include "IgSKI_InputLink.h"
#include "IgSKI_InputProducer.h"
#include "IgSKI_MatchSet.h"
#include "IgSKI_Release.h"
#include "IgSKI_Parser.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_Symbol.h"
#include "IgSKI_SymbolFactory.h"
#include "IgSKI_Agent.h"
#include "IgSKI_Match.h"
#include "IgSKI_Test.h"
#include "IgSKI_TestSet.h"
#include "IgSKI_Wme.h"
#include "IgSKI_WmeReplacementPolicy.h"
#include "IgSKI_State.h"
#include "IgSKI_WMObject.h"
#include "IgSKI_WMStaticView.h"
#include "IgSKI_WorkingMemoryView.h"
#include "IgSKI_RhsAction.h"
#include "IgSKI_ActionElement.h"
#include "IgSKI_RhsFunctionAction.h"

#include "Win32LinuxDefines.h"

namespace gSKI {

   const unsigned short MajorVersionNumber = 0;
   const unsigned short MinorVersionNumber = 1;
   const unsigned short MicroVersionNumber = 0;

}

#endif // GSKI_HEADER
