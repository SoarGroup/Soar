
#ifndef INPUT_CONSOLE
#define INPUT_CONSOLE

#include "Input_Type.h"

class Input_Console : public Input_Type
{
public:

	// named constructor pattern is used to guarantee use of Smart_Pointers
	static Smart_Pointer<Input_Type> create();

	// override get_command pure virtual function. This will just return the string from
	// the command line
	std::string get_command();

	// return true if this is an input file and it is okay to pause it
	virtual bool pausible()
	{ return false; }

	virtual bool killable()
	{ return false; }

private:

	// keep constructor private to guarantee use of named constructor
	Input_Console() {}

};

#endif


