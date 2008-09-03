#include "sml_ClientInputLink.h"
#include "sml_ClientWorkingMemory.h"
#include "sml_Connection.h"
#include "sml_ClientWMObject.h"
#include "sml_ClientIInputProducer.h"
#include "sml_ClientSML.h"

using namespace sml;

InputLink::InputLink(char const* pID, ClientSML* pClientSML)
{
	SetId(pID) ;
	SetClientSML(pClientSML) ;
	m_pWorkingMemory = 0 ;
}

InputLink::~InputLink()
{
	delete m_pWorkingMemory ;
}

IWorkingMemory* InputLink::GetInputLinkMemory(gSKI::Error*)
{
	// Only make the call to get this value if we have no cached value.
	if (!m_pWorkingMemory)
	{
		AnalyzeXML response ;
		
		if (GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IInputLink_GetInputLinkMemory, GetId()))
		{
			m_pWorkingMemory = new WorkingMemory(response.GetResultString(), GetClientSML()) ;
		}
	}

	return m_pWorkingMemory ;
}

void InputLink::AddInputProducer(IWMObject *object, IInputProducer* producer, gSKI::Error* err)
{
	// We need to create an id for this producer (which is a client created object)
	char id[20] ;
	GetClientSML()->GenerateID(producer, id) ;
	producer->SetId(id) ;
	producer->SetClientSML(GetClientSML()) ;

	AnalyzeXML response ;
	GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IInputLink_AddInputProducer,
			GetId(),
			sml_Names::kParamWmeObject, object->GetId(),
			sml_Names::kParamInputProducer, producer->GetId()) ;

	// We also need to record this input producer so we can call back to it when we get a message
	// back from the kernel.
	GetClientSML()->RecordInputProducer(producer) ;
}

void InputLink::GetRootObject(sml::IWMObject** ppObject, gSKI::Error* err)
{
	AnalyzeXML response ;
	*ppObject = NULL ;
	
	if (GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IInputLink_GetRootObject, GetId()))
	{
		*ppObject = new WMObject(response.GetResultString(), GetClientSML()) ;
	}
}
