#include <portability.h>

/////////////////////////////////////////////////////////////////
// InputListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : May 2007
//
// This class's OnKernelEvent method is called when
// the agent's input phase callback is called.
//
/////////////////////////////////////////////////////////////////

#include "sml_InputListener.h"

#include "sml_Utils.h"
#include "sml_Connection.h"
#include "sml_AgentSML.h"
#include "sml_KernelSML.h"
#include "sml_StringOps.h"

#include "KernelHeaders.h"

using namespace sml ;

// Flag to control printing debug information about the input link
#ifdef _DEBUG
	static bool kDebugInput = false ;
#else
	static bool kDebugInput = false ;
#endif

void InputListener::Init(KernelSML* pKernelSML, AgentSML* pAgentSML)
{
	m_KernelSML = pKernelSML ;
	SetAgentSML(pAgentSML) ;
}

// Called when an event occurs in the kernel
void InputListener::OnKernelEvent(int eventID, AgentSML* pAgentSML, void* pCallData)
{
	switch (eventID) {
		case smlEVENT_INPUT_PHASE_CALLBACK:
		{
			int callbacktype = static_cast<int>(reinterpret_cast<intptr_t>(pCallData));

			switch(callbacktype) {
			case TOP_STATE_JUST_CREATED:
			  ProcessPendingInput(pAgentSML, callbacktype) ;
			  break;
			case NORMAL_INPUT_CYCLE:
			  ProcessPendingInput(pAgentSML, callbacktype) ;
			  if (pAgentSML->ReplayQuery())
			  {
				  pAgentSML->ReplayInputWMEs();
			  }
			  break;
			case TOP_STATE_JUST_REMOVED:
			  break;
			}
		}
	} ;
}

void InputListener::ProcessPendingInput(AgentSML* pAgentSML, int )
{
	PendingInputList* pPending = pAgentSML->GetPendingInputList() ;

	bool ok = true ;

	for (PendingInputListIter iter = pPending->begin() ; iter != pPending->end() ; iter = pPending->erase(iter)) {
		soarxml::ElementXML* pInputMsg = *iter ;

		// Analyze the message and find important tags
		AnalyzeXML msg ;
		msg.Analyze(pInputMsg) ;

		// Get the "name" attribute from the <command> tag
		char const* pCommandName = msg.GetCommandName() ;

		// Only input commands should be stored in the pending input list
		(void)pCommandName; // silences warning in release mode
		assert(!strcmp(pCommandName, "input")) ;

		soarxml::ElementXML const* pCommand = msg.GetCommandTag() ;

		int nChildren = pCommand->GetNumberChildren() ;

		soarxml::ElementXML wmeXML(NULL) ;
		soarxml::ElementXML* pWmeXML = &wmeXML ;

		if (kDebugInput)
			PrintDebugFormat("--------- %s starting input ----------", pAgentSML->GetName()) ;

		for (int i = 0 ; i < nChildren ; i++)
		{
			pCommand->GetChild(&wmeXML, i) ;

			// Ignore tags that aren't wmes.
			if (!pWmeXML->IsTag(sml_Names::kTagWME))
				continue ;

			// Find out if this is an add or a remove
			char const* pAction = pWmeXML->GetAttribute(sml_Names::kWME_Action) ;

			if (!pAction)
				continue ;

			bool add = IsStringEqual(pAction, sml_Names::kValueAdd) ;
			bool remove = IsStringEqual(pAction, sml_Names::kValueRemove) ;

			if (add)
			{
				char const* pID			= pWmeXML->GetAttribute(sml_Names::kWME_Id) ;	// May be a client side id value (e.g. "o3" not "O3")
				char const* pAttribute  = pWmeXML->GetAttribute(sml_Names::kWME_Attribute) ;
				char const* pValue		= pWmeXML->GetAttribute(sml_Names::kWME_Value) ;
				char const* pType		= pWmeXML->GetAttribute(sml_Names::kWME_ValueType) ;	// Can be NULL (=> string)
				char const* pTimeTag	= pWmeXML->GetAttribute(sml_Names::kWME_TimeTag) ;	// May be a client side time tag (e.g. -3 not +3)

				// Set the default value
				if (!pType)
					pType = sml_Names::kTypeString ;

				// Check we got everything we need
				if (!pID || !pAttribute || !pValue || !pTimeTag)
					continue ;

				if (kDebugInput)
				{
					PrintDebugFormat("%s Add %s ^%s %s (type %s tag %s)", pAgentSML->GetName(), pID, pAttribute, pValue, pType, pTimeTag) ;
				}

				// Add the wme
				ok = pAgentSML->AddInputWME( pID, pAttribute, pValue, pType, pTimeTag) && ok ;
			}
			else if (remove)
			{
				char const* pTimeTag = pWmeXML->GetAttribute(sml_Names::kWME_TimeTag) ;	// May be (will be?) a client side time tag (e.g. -3 not +3)

				if (kDebugInput)
				{
					PrintDebugFormat("%s Remove tag %s", pAgentSML->GetName(), pTimeTag) ;
				}

				// Remove the wme
				ok = pAgentSML->RemoveInputWME(pTimeTag) && ok ;
			}
		}

		delete pInputMsg ;
	}

	std::list<DirectInputDelta>* pBufferedDirect = pAgentSML->GetBufferedDirectList();
	for (std::list<DirectInputDelta>::iterator iter = pBufferedDirect->begin() ; iter != pBufferedDirect->end() ; iter = pBufferedDirect->erase(iter)) 
	{
		DirectInputDelta& delta = *iter;
		switch (delta.type)
		{
		case DirectInputDelta::kRemove:
			pAgentSML->RemoveInputWME(delta.clientTimeTag);
			break;
		case DirectInputDelta::kAddString:
			pAgentSML->AddStringInputWME(delta.id.c_str(), delta.attribute.c_str(), delta.svalue.c_str(), delta.clientTimeTag);
			break;
		case DirectInputDelta::kAddInt:
			pAgentSML->AddIntInputWME(delta.id.c_str(), delta.attribute.c_str(), delta.ivalue, delta.clientTimeTag);
			break;
		case DirectInputDelta::kAddDouble:
			pAgentSML->AddDoubleInputWME(delta.id.c_str(), delta.attribute.c_str(), delta.dvalue, delta.clientTimeTag);
			break;
		case DirectInputDelta::kAddId:
			pAgentSML->AddIdInputWME(delta.id.c_str(), delta.attribute.c_str(), delta.svalue.c_str(), delta.clientTimeTag);
			break;
		default:
			assert(false);
			break;
		}
	}
}

// Register for the events that KernelSML itself needs to know about in order to work correctly.
void InputListener::RegisterForKernelSMLEvents()
{
	// Listen for input callback events so we can submit pending input to the kernel
	this->RegisterWithKernel(smlEVENT_INPUT_PHASE_CALLBACK) ;
}

void InputListener::UnRegisterForKernelSMLEvents()
{
	this->UnregisterWithKernel(smlEVENT_INPUT_PHASE_CALLBACK) ;
}

