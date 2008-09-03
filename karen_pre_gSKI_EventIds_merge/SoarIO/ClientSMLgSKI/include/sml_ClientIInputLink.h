#ifndef SML_CLIENT_IINPUTLINK
#define SML_CLIENT_IINPUTLINK

#include "sml_ClientObject.h"

namespace sml
{
class IWorkingMemory;
class IWMObject ;
class IInputProducer ;

class IInputLink : public ClientObject
{
public:
	virtual ~IInputLink() { } ;

	virtual sml::IWorkingMemory* GetInputLinkMemory(gSKI::Error* err = 0) = 0;
	virtual void AddInputProducer(sml::IWMObject *object, sml::IInputProducer* producer, gSKI::Error* err = 0) = 0;
	virtual void GetRootObject(sml::IWMObject** ppObject, gSKI::Error* err = 0) = 0;
};

}//closes namespace

#endif//SML_CLIENT_IINPUTLINK