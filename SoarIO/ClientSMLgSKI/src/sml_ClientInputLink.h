#ifndef SML_CLIENT_INPUTLINK
#define SML_CLIENT_INPUTLINK

#include "sml_ClientIInputLink.h"

namespace sml
{

class InputLink : public IInputLink
{
protected:
	IWorkingMemory*		m_pWorkingMemory ;

public:
	InputLink(char const* pID, ClientSML* pClientSML) ;
	virtual ~InputLink() ;

	sml::IWorkingMemory* GetInputLinkMemory(gSKI::Error* err = 0);
	void AddInputProducer(sml::IWMObject *object, sml::IInputProducer* producer, gSKI::Error* err = 0) ;
	void GetRootObject(sml::IWMObject** ppObject, gSKI::Error* err = 0) ;
};

}//closes namespace

#endif//SML_CLIENT_INPUTLINK