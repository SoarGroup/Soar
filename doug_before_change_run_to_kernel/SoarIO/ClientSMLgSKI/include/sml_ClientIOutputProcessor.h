
#ifndef CLIENT_IOUTPUT_PROCESSOR_H
#define CLIENT_IOUTPUT_PROCESSOR_H

#include "sml_ClientObject.h"

namespace sml {

// Forward
class IWorkingMemory ;
class IWMObject ;

class IOutputProcessor : public ClientObject
{
public:
	IOutputProcessor(void);
	virtual ~IOutputProcessor(void);

	virtual void ProcessOutput(IWorkingMemory*, IWMObject*) = 0 ;
};

}	// End of namespace

#endif // IOUTPUT_PROCESSOR_H