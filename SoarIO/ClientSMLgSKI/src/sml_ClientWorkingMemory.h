#ifndef SML_CLIENT_WORKINGMEMORY
#define SML_CLIENT_WORKINGMEMORY

#include "gSKI_Structures.h"
#include "sml_ClientIWorkingMemory.h"

namespace sml
{

//will have 3 subsets: input, output, and main  - otherwise make 3 subclasses
	class WorkingMemory : public IWorkingMemory
{
protected:
	IAgent*		m_pAgent ;

public:
	WorkingMemory(char const* pID, ClientSML* pClientSML) ;
	virtual ~WorkingMemory() ;

	virtual sml::IWme* ReplaceStringWme(sml::IWme* oldWme, const char* newValue, gSKI::Error* err = 0);

	virtual sml::IWme* AddWmeNewObject(sml::IWMObject* wmObject,
                                   const char* attr,
								   gSKI::Error* err = 0);
	
	virtual sml::IWme* AddWmeString(sml::IWMObject* wmObject,
                                const char* attr,
                                const char* value,
								gSKI::Error* err = 0);

	virtual sml::IWme* AddWmeInt(sml::IWMObject* wmObject,
                             const char* attr,
                             int intValue,
							 gSKI::Error* err = 0);

	virtual void RemoveObject(sml::IWMObject* object, gSKI::Error* err = 0);

	virtual sml::IAgent* GetAgent(gSKI::Error* err = 0);
};

}//closes namespace


#endif //SML_CLIENT_WORKINGMEMORY