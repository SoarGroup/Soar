/////////////////////////////////////////////////////////////////
// Connection class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// This class represents a logical connection between two entities that are communicating via SML.
// For example, an environment (the client) and the Soar kernel.
//
// The connection can be "embedded" which means both the client and the kernel are in the same process
// or it can be "remote" which means the client and the kernel are in different processes and possibly on different machines.
//
// Commands formatted as SML (a form of XML) are sent over this connection to issue commands etc.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_CONNECTION_H
#define SML_CONNECTION_H

namespace sml
{

typedef ErrorCode int ;

class Connection
{
public:
	Connection(void);
	virtual ~Connection(void);
};

} // End of namespace

#endif // SML_CONNECTION_H
