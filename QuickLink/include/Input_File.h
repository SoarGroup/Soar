/* Input_File.h
*
* This class inherits from the Input_Type abstract base class.
* An Input_File object should be created for any file read that will
* read properly formatted QL commands such as a 'load' or a 'loadp' call
*
*/

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

	// return true if this can be terminated before it finishes, this is true for file inputs.
	virtual bool killable()
	{ return true; }

	~Input_File();

private:

	// keep constructor private to guarantee use of named constructor
	Input_File(std::string filename);
	
	// the source of the input
	std::ifstream infile;
};

#endif



