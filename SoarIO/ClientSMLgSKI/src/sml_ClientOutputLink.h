#ifndef SML_CLIENT_OUTPUTLINK
#define SML_CLIENT_OUTPUTLINK

#include "sml_ClientIOutputLink.h"

namespace sml
{
class IWorkingMemory;
class IWMObject ;
class IOutputProcessor ;

class OutputLink : public IOutputLink
{
protected:
	IWorkingMemory*		m_pWorkingMemory ;

public:
	OutputLink(char const* pID, ClientSML* pClientSML) ;
	virtual ~OutputLink() ;

	sml::IWorkingMemory* GetOutputLinkMemory(gSKI::Error* err = 0);
	virtual void AddOutputProcessor(char const* pAttributePath, sml::IOutputProcessor* producer, gSKI::Error* err = 0) ;
	void GetRootObject(sml::IWMObject** ppObject, gSKI::Error* err = 0) ;
	void SetAutomaticUpdate(bool value, gSKI::Error* err = 0) ;
};

}//closes namespace

#endif//SML_CLIENT_OUTPUTLINK