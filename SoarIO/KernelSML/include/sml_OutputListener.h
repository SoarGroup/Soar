/////////////////////////////////////////////////////////////////
// OutputListener class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : September 2004
//
// This class's HandleEvent method is called when
// the agent adds wmes to the output link.
//
/////////////////////////////////////////////////////////////////
#ifndef OUTPUT_LISTENER_H
#define OUTPUT_LISTENER_H

#include "gSKI_Events.h"
#include "gSKI_Enumerations.h"
#include "IgSKI_Iterator.h"
#include "IgSKI_Agent.h"

namespace sml {

class KernelSML ;
class Connection ;

class OutputListener : public gSKI::IWorkingMemoryListener
{
protected:
	KernelSML*	m_KernelSML ;
	Connection* m_Connection ;

public:
	OutputListener(KernelSML* pKernelSML, Connection* pConnection)
	{
		m_KernelSML = pKernelSML ;
		m_Connection = pConnection ;
	}

	virtual void HandleEvent(egSKIEventId eventId, gSKI::IAgent* agentPtr, egSKIWorkingMemoryChange change, gSKI::tIWmeIterator* wmelist) ;

} ;

}

#endif
