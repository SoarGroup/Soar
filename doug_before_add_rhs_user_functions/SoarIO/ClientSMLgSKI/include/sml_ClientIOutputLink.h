#ifndef SML_CLIENT_IOUTPUTLINK
#define SML_CLIENT_IOUTPUTLINK

#include "sml_ClientObject.h"

namespace sml
{
class IWorkingMemory;
class IWMObject ;
class IOutputProcessor ;

class IOutputLink : public ClientObject
{
public:
	virtual ~IOutputLink() { } ;

	virtual sml::IWorkingMemory* GetOutputLinkMemory(gSKI::Error* err = 0) = 0;
	virtual void AddOutputProcessor(char const* pAttributePath, sml::IOutputProcessor* producer, gSKI::Error* err = 0) = 0;
	virtual void GetRootObject(sml::IWMObject** ppObject, gSKI::Error* err = 0) = 0 ;
	virtual	void SetAutomaticUpdate(bool value, gSKI::Error* err = 0) = 0 ;

};

}//closes namespace

#endif//SML_CLIENT_OUTPUTLINK