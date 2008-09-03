//////////////////////////////////////////////////////////////////////
// ClientSML class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This class owns the Connection* used to communicate with the Soar
// kernel and could retain pointers to other objects (e.g. IKernel)
// if that proves useful.
//
//////////////////////////////////////////////////////////////////////

#ifndef SML_CLIENT_SML_H
#define SML_CLIENT_SML_H

#include "sml_IdMap.h"

namespace sml {

class Connection ;
class ElementXML ;
class AnalyzeXML ;
class IInputProducer ;
class IOutputProcessor ;
class IWorkingMemory ;

class ClientSML
{
protected:
	Connection*	m_Connection ;

	// We need to keep track of all input producers and output processors
	// so when Soar calls to them, we can route the callback to the correct place.
	IdMap		m_InputProducers ;
	IdMap		m_OutputProcessors ;

	// We also need to keep track of the working memory objects
	// so that we can pass in the correct object to input producers/output processors
	IdMap		m_WorkingMemories ;

	void RecordWorkingMemory(IWorkingMemory* pWM) ;

	void InputProducerUpdate(Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse) ;
	void OutputProcessorUpdate(Connection* pConnection, AnalyzeXML* pIncoming, ElementXML* pResponse) ;

public:
	ClientSML(void);
	~ClientSML(void);

	char* GenerateID(void const* pObject, char* pBuffer)
	{
		// Create a hex id string
		sprintf(pBuffer, "0x%x", (int)pObject) ;

		// Return the ID
		return pBuffer ;
	}

	void RecordInputProducer(IInputProducer* pInput) ;
	void RecordOutputProcessor(IOutputProcessor* pOutput) ;

	// This gets called when we get a message from the kernel
	ElementXML* ReceivedCallFromKernel(Connection* pConnection, ElementXML* pIncomingMsg) ;

	void SetConnection(Connection* pConnection) { m_Connection = pConnection ; }
	Connection* GetConnection()	const			{ return m_Connection ; }
};

}	// end namespace

#endif // SML_CLIENT_SML_H