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
// This class is actually an abstract base class, with specific implementations of this class
// being used to provide the different types of connections.
//
/////////////////////////////////////////////////////////////////

#include "sml_Connection.h"

using namespace sml ;

