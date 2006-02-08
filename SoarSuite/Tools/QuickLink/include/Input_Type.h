
#ifndef INPUT_TYPE
#define INPUT_TYPE

#include "Smart_Pointer.h"

#include <string>

class Input_Type : public Reference_Counted_Object
{
public:

	// return a string form of the command 
	virtual std::string get_command() = 0;

	// return true if this is an input file and it is okay to pause it
	virtual bool pausible() = 0;

	// this should return true if it is possible to end the child type of this before it finishes
	virtual bool killable() = 0;

	// virtual destructor
	virtual ~Input_Type() {}

	

protected:

	// keep the constructor protected so only derived classes have access to it
	Input_Type() {}
};

#endif