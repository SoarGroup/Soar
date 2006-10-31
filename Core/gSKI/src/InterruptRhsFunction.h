/*************************************************************************
* PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
* FOR LICENSE AND COPYRIGHT INFORMATION. 
*************************************************************************/

// InterruptRhsFunction.h
#ifndef _INTERRUPT_RHS_FUNCTION_H_
#define _INTERRUPT_RHS_FUNCTION_H_

/*
A simple rhs function for interrupting all running agents 
when a production is fired.
*/

#include "gSKI_RhsFunction.h"
#include "gSKI_AgentManager.h"

class InterruptRhsFunction: public gSKI::RhsFunction
{
public:
	InterruptRhsFunction(gSKI::AgentManager* manager):m_manager(manager) { }

	const char* GetName() const { return "interrupt"; }
	int GetNumExpectedParameters() const { return 0; }
	bool IsValueReturned() const { return false; }

	gSKI::ISymbol* Execute(gSKI::tISymbolIterator* pArguments,
		gSKI::ISymbolFactory* pSymFactory) {
			// unreferenced formal parameters
			(void)(pArguments);
			(void)(pSymFactory);

			m_manager->InterruptAll(gSKI_STOP_AFTER_SMALLEST_STEP);

			return 0;
	}

private:
	gSKI::AgentManager* m_manager;
};

#endif
