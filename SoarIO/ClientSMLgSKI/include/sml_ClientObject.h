/////////////////////////////////////////////////////////////////
// ClientObject class
//
// Author: Douglas Pearson	www.threepenny.net
// Date  : August 2004
//
// This is the base class for all SML Client objects.
// The class is a thin wrapper around an "id" which refers to
// a specific gSKI object.  KernelSML maps between these id's
// and the actual gSKI object as messages are sent to it.
//
/////////////////////////////////////////////////////////////////

#include "sml_ClientError.h"
#include "gSKI_ClientEnumerations.h"
#include "gSKI_ClientStructures.h"

#ifndef SML_CLIENT_OBJECT_H
#define SML_CLIENT_OBJECT_H

#include <string>

namespace sml
{
class ClientSML ;
class Connection ;

class ClientObject : public sml_ClientError
{
public:
	char const* GetId() const { return id.c_str(); };

	Connection* GetConnection() const ;

	ClientSML*	GetClientSML() const { return m_ClientSML ; }

	virtual ~ClientObject() { } 

	// BADBAD: We should make these protected and then have IInputProducer
	// and IOutputProducer be friends.
	void SetId(char const* pID) { if (pID != NULL) id = pID ;}

	void SetClientSML(ClientSML* pClientSML) { m_ClientSML = pClientSML ; }

protected:
	ClientObject(const std::string& inId) : id(inId) {}

	//is used in the case that a subclass doesn't know its name at the time of construction
	ClientObject(){}

private:
	std::string id;

	ClientSML*	m_ClientSML ;
};

}//closes namespace

#endif // SML_CLIENT_OBJECT_H
