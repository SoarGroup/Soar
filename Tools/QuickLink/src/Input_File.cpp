
#include "Input_File.h"
#include "Utilities.h"
#include "QL_Interface.h"
#include "Input_Controller.h"

#include <string>
#include <iostream>

using std::string;

// named constructor
Smart_Pointer<Input_Type> Input_File::create(const string& filename)
{
	Smart_Pointer<Input_File> ptr = new Input_File(filename);
	return ptr;
}

// called by named constructor, opens file and turns off QL updates so that all
// of the file commands do not generate output
Input_File::Input_File(string filename) 
: infile(filename.c_str())
{
	if(!infile)
		throw Error("The file: " + filename + " could not be opened");
	QL_Interface::instance().turn_off_updates(); // turn off view updates
}

// destructor
Input_File::~Input_File()
{
	if(!Input_Controller::instance().is_shutting_down())
	{
		QL_Interface::instance().turn_on_updates(); // turn updates back on
		QL_Interface::instance().update_views_now(); // show the current state
	}
}


// returns the next line of command in the file
string Input_File::get_command()
{
	string command;
	while(true)
	{
		getline(infile, command);

		// if file is empty, return an empty string to signal end of file
		if(!infile)
		{
			command = "";
			break;
		}

		// if the file is not a comment and is not empty, return it
		if(command.length() && command[0] != '#')
			break;
	}
	
	return command;
}






