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

#include "sml_OutputListener.h"
#include "sml_Connection.h"

using namespace sml ;

void OutputListener::HandleEvent(egSKIEventId eventId, gSKI::IAgent* agentPtr, egSKIWorkingMemoryChange change, gSKI::tIWmeIterator* wmelist)
{
	AnalyzeXML response ;

	// Build an output command and send it back to the client
	// I'm not sure what to put in the command.
	// Could build a tree from the top of the o-link.  That seems the most helpful and mark the wmes that have changed with "changed" actions.
	// If so, need to include a depth test, so if change wme at depth 4 nothing is posted.
	// Should think about how the client will handle the message once it comes in.
	// If it's already in tree form within the message the client side will be trivial.  GetNumberChildren().  GetWme(int i).  IsIdentifier().  Repeat.
	// <wme-id id="O3" att="list" value="O6">
	//     <wme id="O6" att="name" value="fred"></wme>
	// </wme-id>

	//	m_Connection->SendAgentCommand(&response, ...) ;


}
