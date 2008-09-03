#include "sml_ClientWorkingMemory.h"
#include "sml_ClientAgent.h"
#include "sml_Connection.h"
#include "sml_ClientWME.h"
#include "sml_ClientRelease.h"
#include "sml_StringOps.h"

using namespace sml;

WorkingMemory::WorkingMemory(char const* pID, ClientSML* pClientSML)
{
	SetId(pID) ;
	SetClientSML(pClientSML) ;
	m_pAgent = 0 ;
}

WorkingMemory::~WorkingMemory()
{
	delete m_pAgent ;
}

sml::IWme* WorkingMemory::ReplaceStringWme(sml::IWme* oldIWme, const char* newValue, gSKI::Error*)
{
	AnalyzeXML response ;
	IWme* pWme = NULL ;

	if (GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IWorkingMemory_ReplaceStringWme, GetId(),
		sml_Names::kParamWme, oldIWme->GetId(), sml_Names::kParamValue, newValue))
	{
		// NOTE: The caller is responsible for deleting this.  This matches gSKI's definition.
		pWme = new WME(response.GetResultString(), GetClientSML()) ;
	}

	return pWme ;
}

sml::IWme* WorkingMemory::AddWmeNewObject(sml::IWMObject* wmObject, const char* attr, gSKI::Error*)
{
	AnalyzeXML response ;
	IWme* pWme = NULL ;

	if (GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IWorkingMemory_AddWmeNewObject, GetId(),
		sml_Names::kParamWmeObject, wmObject->GetId(), sml_Names::kParamAttribute, attr))
	{
		// NOTE: The caller is responsible for deleting this.  This matches gSKI's definition.
		pWme = new WME(response.GetResultString(), GetClientSML()) ;
	}

	return pWme ;
}

sml::IWme* WorkingMemory::AddWmeString(sml::IWMObject* wmObject, const char* attr, const char* value,
																							gSKI::Error*)
{
	AnalyzeXML response ;
	IWme* pWme = NULL ;

	if (GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IWorkingMemory_AddWmeString, GetId(),
		sml_Names::kParamWmeObject, wmObject->GetId(), sml_Names::kParamAttribute, attr, sml_Names::kParamValue, value))
	{
		// NOTE: The caller is responsible for deleting this.  This matches gSKI's definition.
		pWme = new WME(response.GetResultString(), GetClientSML()) ;
	}

	return pWme ;
}

sml::IWme* WorkingMemory::AddWmeInt(sml::IWMObject* wmObject, const char* attr, int intValue, gSKI::Error*)
{
	AnalyzeXML response ;
	IWme* pWme = NULL ;

	char value[kMinBufferSize] ;
	Int2String(intValue, value, sizeof(value)) ;

	if (GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IWorkingMemory_AddWmeInt, GetId(),
		sml_Names::kParamWmeObject, wmObject->GetId(), sml_Names::kParamAttribute, attr, sml_Names::kParamValue, value))
	{
		// NOTE: The caller is responsible for deleting this.  This matches gSKI's definition.
		pWme = new WME(response.GetResultString(), GetClientSML()) ;
	}

	return pWme ;
}

void WorkingMemory::RemoveObject(sml::IWMObject* wmObject, gSKI::Error* err)
{
	AnalyzeXML response ;

	GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IWorkingMemory_RemoveObject, GetId(),
		sml_Names::kParamWmeObject, wmObject->GetId()) ;
}

IWme* WorkingMemory::AddWmeObjectLink(IWMObject* wmObject, const char* attr, IWMObject* value, gSKI::Error* err)
{
	AnalyzeXML response ;

	IWme* pValue = NULL ;

	if (GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IWorkingMemory_AddWmeObjectLink, GetId(),
		sml_Names::kParamWmeObject, wmObject->GetId(),
		sml_Names::kParamAttribute, attr,
		sml_Names::kParamValue, value->GetId()))
	{
		pValue = new WME(response.GetResultString(), GetClientSML()) ;
	}

	return pValue ;
}


sml::IAgent* WorkingMemory::GetAgent(gSKI::Error*)
{
	if (!m_pAgent)
	{
		AnalyzeXML response ;
		
		if (GetConnection()->SendClassCommand(&response, sml_Names::kgSKI_IWorkingMemory_GetAgent, GetId()))
		{
			m_pAgent = new Agent(response.GetResultString(), GetClientSML()) ;
		}
	}

	return m_pAgent ;
}
