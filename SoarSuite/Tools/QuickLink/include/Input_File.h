
#ifndef INPUT_FILE
#define INPUT_FILE

#include "Input_Type.h"

#include <fstream>

class Input_File : public Input_Type
{
public:

	// named constructor pattern is used to guarantee use of Smart_Pointers
	static Smart_Pointer<Input_Type> create(const std::string& filename);

	// override get_command pure virtual function. This will just return the string from
	// the command line
	virtual std::string get_command();

	// return true if this is an input file and it is okay to pause it
	virtual bool pausible()
	{ return true; }

	virtual bool killable()
	{ return true; }

	~Input_File();

private:

	// keep constructor private to guarantee use of named constructor
	Input_File(std::string filename);


	std::ifstream infile;
};

#endif



