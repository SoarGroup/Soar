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

#ifndef INPUT_LISTENER_H
#define INPUT_LISTENER_H

#include "sml_KernelCallback.h"

#include <map>

namespace sml {

class KernelSML ;

class InputListener : public KernelCallback
{
protected:
	KernelSML*		m_KernelSML ;

	void ProcessPendingInput(AgentSML* pAgentSML, int callbacktype) ;

public:
	InputListener()
	{
		m_KernelSML = 0 ;
	}

	virtual ~InputListener()
	{
	}

	void Init(KernelSML* pKernelSML, AgentSML* pAgentSML);

	// Called when an event occurs in the kernel
	virtual void OnKernelEvent(int eventID, AgentSML* pAgentSML, void* pCallData) ;

	// Register for the events that KernelSML itself needs to know about in order to work correctly.
	void RegisterForKernelSMLEvents() ;

	// UnRegister for the events that KernelSML itself needs to know about in order to work correctly.
	void UnRegisterForKernelSMLEvents() ;
} ;

}

#endif
