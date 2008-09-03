#ifndef SML_CLIENT_IWORKINGMEMORY
#define SML_CLIENT_IWORKINGMEMORY

#include "sml_ClientObject.h"


namespace sml
{
class IWme;
class IAgent;
class IWMObject;

//will have 3 subsets: input, output, and main  - otherwise make 3 subclasses
class IWorkingMemory : public ClientObject
{
public:
	virtual ~IWorkingMemory() { } ;

	virtual sml::IWme* ReplaceStringWme(sml::IWme* oldWme, const char* newValue, gSKI::Error* err = 0) = 0;

	virtual sml::IWme* AddWmeNewObject(sml::IWMObject* wmObject,
                                   const char* attr,
								   gSKI::Error* err = 0) = 0;
	
	virtual sml::IWme* AddWmeString(sml::IWMObject* wmObject,
                                const char* attr,
                                const char* value,
								gSKI::Error* err = 0) = 0;

	virtual sml::IWme* AddWmeInt(sml::IWMObject* wmObject,
                             const char* attr,
                             int intValue,
							 gSKI::Error* err = 0) = 0;

	virtual void RemoveObject(sml::IWMObject* object, gSKI::Error* err = 0) = 0;

	virtual IWme* AddWmeObjectLink(IWMObject* wmObject, const char* attr, IWMObject* value, gSKI::Error* err = 0) ;

	virtual sml::IAgent* GetAgent(gSKI::Error* err = 0) = 0;
};

}//closes namespace


#endif //SML_CLIENT_WORKINGMEMORY