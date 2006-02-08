
#include "Input_Controller.h"
#include "Input_Console.h"
#include "Input_File.h"

#include <cassert>

using std::string;

Input_Controller& Input_Controller::instance()
{
	static Input_Controller IC;
	return IC;
}

Input_Controller::Input_Controller() : shutting_down(false)
{
	// the last element in the input stack should always be the console input
	input_stack.push_front(Input_Console::create());
}

void Input_Controller::generate_new_input(INPUT_TYPE_T e_input, string filename)
{
	switch(e_input) 
	{
		case INPUT_FILE_T :
			input_stack.push_front(Input_File::create(filename.c_str()));
			break;
		default :
			assert(false && "This case is not implemented");
	}
}

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

string Input_Controller::force_command_line_input()
{
	string command;
	// we are guaranteed that the last object is input from the command line
	command = (*(input_stack.rbegin()))->get_command();
	return command;
}

bool Input_Controller::okay_to_pause()
{
	return (*(input_stack.begin()))->pausible();
}

void Input_Controller::kill_current_input_source()
{
	if((*(input_stack.begin()))->killable())
		input_stack.pop_front();
}

void Input_Controller::IC_Shutdown()
{
	shutting_down = true;
	input_stack.clear();
}

bool Input_Controller::is_shutting_down()
{
	return shutting_down;
}


