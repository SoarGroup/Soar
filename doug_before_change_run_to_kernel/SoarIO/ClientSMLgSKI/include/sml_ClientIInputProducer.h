
#ifndef CLIENT_IINPUT_PRODUCER_H
#define CLIENT_IINPUT_PRODUCER_H

#include "sml_ClientObject.h"

namespace sml {

// Forward
class IWorkingMemory ;
class IWMObject ;

class IInputProducer : public ClientObject
{
public:
	IInputProducer(void);
	virtual ~IInputProducer(void);

	virtual void Update(IWorkingMemory*, IWMObject*) = 0 ;
};

}	// End of namespace

#endif // IINPUT_PRODUCER_H