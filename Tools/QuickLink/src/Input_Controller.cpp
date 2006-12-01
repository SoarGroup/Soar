
#include "Input_Controller.h"
#include "Input_Console.h"
#include "Input_File.h"

#include <cassert>

using std::string;

// returns an instance of the singleton class
Input_Controller& Input_Controller::instance()
{
	static Input_Controller IC;
	return IC;
}

// constructor creates the input_console object and pushes it on input stack
Input_Controller::Input_Controller() : should_print_prompt(true), shutting_down(false)
{
	// the last element in the input stack should always be the console input
	input_stack.push_front(Input_Console::create());
}

// called by QL_Controller when a file for input needs to be opened
void Input_Controller::generate_new_input(INPUT_TYPE_T e_input, string filename)
{
	// based on the input type, create a new input source and put it on the stack
	switch(e_input) 
	{
		case INPUT_FILE_T :
			// create the object and push it on the input stack so that it will be read
			input_stack.push_front(Input_File::create(filename.c_str()));
			break;
		default :
			assert(false && "This case is not implemented");
	}
}

// returns a command from the input source on the top of the stack
string Input_Controller::get_command()
{
	string command;
	while(true)
	{
		command = (*(input_stack.begin()))->get_command();

		// if the command call returns something other than the empty string, return it
		if(command != "")
			break;
		 // update the input stack
		if(input_stack.size() != 1)
		{
			input_stack.pop_front();
			assert(input_stack.size() > 0);
		}
	}

	return command;
	
}

// sometimes, during a pause of a file reading, input from console can come in, this handles
// that case
string Input_Controller::force_command_line_input()
{
	string command;
	// we are guaranteed that the last object is input from the command line
	command = (*(input_stack.rbegin()))->get_command();
	return command;
}

// check to see if the current source is pausible
bool Input_Controller::okay_to_pause()
{
	return (*(input_stack.begin()))->pausible();
}

// a process can be terminated before it finishes, this accomplishes that
void Input_Controller::kill_current_input_source()
{
	if((*(input_stack.begin()))->killable())
		input_stack.pop_front();
}

// call at end of input use
void Input_Controller::IC_Shutdown()
{
	shutting_down = true;
	input_stack.clear();
}

// needed by input_type children to know what needs to be accomplished when shutting down
bool Input_Controller::is_shutting_down()
{
	return shutting_down;
}


