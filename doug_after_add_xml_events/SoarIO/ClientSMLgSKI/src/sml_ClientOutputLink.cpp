#include "sml_ClientOutputLink.h"
#include "sml_ClientWorkingMemory.h"
#include "sml_Connection.h"
#include "sml_ClientWMObject.h"
#include "sml_ClientIOutputProcessor.h"
#include "sml_ClientSML.h"

using namespace sml;

OutputLink::OutputLink(char const* pID, ClientSML* pClientSML)
{
	SetId(pID) ;
	SetClientSML(pClientSML) ;
	m_pWorkingMemory = 0 ;
}

OutputLink::~OutputLink()
{
	delete m_pWorkingMemory ;
}

IWorkingMemory* OutputLink::GetOutputLinkMemory(gSKI::Error*)
{
	// Only make the call to get this value if we have no cached value.
	if (!m_pWorkingMemory)
	{
		AnalyzeXML response ;
		
		if (GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IOutputLink_GetOutputLinkMemory, GetId()))
		{
			m_pWorkingMemory = new WorkingMemory(response.GetResultString(), GetClientSML()) ;
		}
	}

	return m_pWorkingMemory ;
}

void OutputLink::AddOutputProcessor(char const* pAttributePath, IOutputProcessor* producer, gSKI::Error* err)
{
	// We need to create an id for this producer (which is a client created object)
	char id[20] ;
	GetClientSML()->GenerateID(producer, id) ;
	producer->SetId(id) ;
	producer->SetClientSML(GetClientSML()) ;

	AnalyzeXML response ;
	GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IOutputLink_AddOutputProcessor,
			GetId(),
			sml_Names::kParamAttributePath, pAttributePath,
			sml_Names::kParamOutputProcessor, producer->GetId()) ;

	// We need to keep track of this so we can handle callbacks from the kernel
	GetClientSML()->RecordOutputProcessor(producer) ;
}

void OutputLink::SetAutomaticUpdate(bool value, gSKI::Error* err)
{
	AnalyzeXML response ;
	GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IOutputLink_SetAutomaticUpdate,
			GetId(),
			sml_Names::kParamValue, value ? sml_Names::kTrue : sml_Names::kFalse) ;
}

void OutputLink::GetRootObject(sml::IWMObject** ppObject, gSKI::Error* err)
{
	AnalyzeXML response ;
	*ppObject = NULL ;

	if (GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IOutputLink_GetRootObject, GetId()))
	{
		*ppObject = new WMObject(response.GetResultString(), GetClientSML()) ;
	}
}
